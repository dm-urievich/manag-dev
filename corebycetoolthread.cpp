#include "corebycetoolthread.h"

CoreByceToolThread::CoreByceToolThread(QObject *parent) :
    QThread(parent)
{
    transferHardwareModules = new TransferThread(&hardwareVector, &lockHardwareVector);
    portSettings = new QSettings("dev-settings.ini", QSettings::IniFormat);

    mbPort = NULL;

    connect(transferHardwareModules, SIGNAL(eventInModule(bool)), this, SLOT(generateXmlHardware(bool)));
    connect(this, SIGNAL(setTransferPeriod(int)), transferHardwareModules, SLOT(setPeriod(int)));
}

void CoreByceToolThread::run()
{
    // заполнить вектор устройствами
    findDevices("config.xml");

    portOpenClose();

    transferHardwareModules->start();

    this->exec();
}

void CoreByceToolThread::portOpenClose(void)
{
    QTextStream cin(stdin), cout(stdout);
    QString portName;
    QString portPath;
    QString parity;
    int baud;
    int dataBit = 1;
    int stopBit;

    portName = portSettings->value("portName").toString();

    if (portName.isEmpty()) {
        cout << "write name of port, like tty*" << endl;
        cin >> portName;
        cout << "write boudrate" << endl;
        cin >> baud;
        cout << "write parity, 'N' - none, 'E' - even, 'O' - odd" << endl;
        cin >> parity;
        cout << "write stop bit's" << endl;
        cin >> stopBit;
    }
    else {
        parity = portSettings->value("parity").toString();
        baud = portSettings->value("baud").toInt();
        stopBit = portSettings->value("stopBits").toInt();
    }

    portPath = "/dev/" + portName;

    cout << "port to be opened: " << portName << endl;
    cout << "boudrate: " << baud << endl;
    cout << "parity: " << parity << endl;
    cout << "stop bit: " << stopBit << endl;

    if (mbPort == NULL) {         // порт закрыт
        mbPort = modbus_new_rtu(portPath.toAscii().constData(), baud, parity.toAscii().at(0), dataBit, stopBit, 0);
        if ( modbus_connect(mbPort) == -1) {
            mbPort = NULL;
            qDebug() << "mbPort not initialized";
        }
        else {
            qDebug() << "mbPort Open " << portPath;

            Hardware::setMbPort(mbPort);
        }
    }
    else {
        modbus_close(mbPort);
        modbus_free(mbPort);
        mbPort = NULL;
        qDebug() << "mbPort Close";

        Hardware::setMbPort(NULL);
    }

    // write to file
    portSettings->setValue("portName", portName);
    portSettings->setValue("baud", baud);
    portSettings->setValue("parity", parity);
    portSettings->setValue("stopBits", stopBit);
}

void CoreByceToolThread::findDevices(QString fileName)
{
    QDomDocument doc("devices");
    QFile inFile(fileName);
    QString errorParse;
    QString tagName;
    int errorLine;

    eSwitch *eswitchDev;
    QString type;
    QString name;
    int addr;
    int id;

    if (!inFile.open(QIODevice::ReadOnly)) {
        qDebug() << "couldn't open file: " << fileName;
        return;
    }
    if (!doc.setContent(&inFile, &errorParse, &errorLine)) {
        qDebug() << "Error: " << errorParse;
        qDebug() << "in line: " << errorLine << endl;
        inFile.close();
        return;
    }
    inFile.close();

    // печатает имена всех непосредственных потомков
    // внешнего элемента.
    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    QDomNode childNod;
    QDomElement childElem;
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // пробуем преобразовать узел в элемент.
        if(!e.isNull()) {
            //qDebug() << e.tagName() << '\n'; // узел действительно является элементом.
            //qDebug() << "attr: " << e.attribute("id") << '\n';
            if (e.tagName() == "objectModule") {
                childNod = e.firstChild();
                while(!childNod.isNull()) {
                    childElem = childNod.toElement();
                    if(!childElem.isNull()) {
                        tagName = childElem.tagName(); // узел действительно является элементом.
                        //qDebug() << tagName;
                        if (tagName == "type") {
                            type = childElem.text();
                        }
                        if (tagName == "mbAddr") {
                            addr = childElem.text().toInt();
                        }
                        if (tagName == "idModule") {
                            id = childElem.text().toInt();
                        }
                        if (tagName == "name") {
                            name = childElem.text();
                        }
                    }
                    childNod = childNod.nextSibling();
                }

                // непосредственное создание устройства
                eswitchDev = new eSwitch(0, name, addr);
                eswitchDev->idModule = id;
                createNewHardwareModule(eswitchDev);
            }

        }
        n = n.nextSibling();
    }
    /*
    QString name = "Device";
    int addr = 1;
    int id = 100500;
    eSwitch *eswitchDev;

    eswitchDev = new eSwitch(0, name, addr);
    eswitchDev->idModule = id;
    createNewHardwareModule(eswitchDev);
    */
}

void CoreByceToolThread::createNewHardwareModule(Hardware *device)
{
    QMutexLocker locker(&lockHardwareVector);

    device->moveToThread(transferHardwareModules);
    hardwareVector.push_back(device);
}

void CoreByceToolThread::generateXmlHardware(bool isEvent)
{
    QVector<Hardware*>::Iterator device;

    QFile outFile("moduleEvents.xml");
    if (outFile.open(QIODevice::WriteOnly)) {
        QTextStream out(&outFile);
        out.setCodec("UTF-8");
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        out << "<module>\n";

        lockHardwareVector.lock();
        for (device = hardwareVector.begin(); device != hardwareVector.end(); ++device) {
            if (isEvent) {
                if ((*device)->isEvent()) {
                    (*device)->generateXml(out);
                }
            }
            else {
                (*device)->generateXml(out);        // для инициализации Gui, генерим состояние всех девайсов
            }
        }
        lockHardwareVector.unlock();

        out << "</module>\n";
        outFile.close();

        emit guiRefresh();
    }
}

void CoreByceToolThread::parseSockets()
{
    QDomDocument doc("module");
    QFile inFile("moduleSockets.xml");
    QString errorParse;
    QVector<Hardware*>::iterator module; //= hardwareVector.begin();
    int errorLine;
    int idModule = 0;

    if (!inFile.open(QIODevice::ReadOnly))
        return;
    if (!doc.setContent(&inFile, &errorParse, &errorLine)) {
        qDebug() << "Error: " << errorParse;
        qDebug() << "in line: " << errorLine << endl;
        inFile.close();
        return;
    }
    inFile.close();

    // печатает имена всех непосредственных потомков
    // внешнего элемента.
    QDomElement docElem = doc.documentElement();

    lockHardwareVector.lock();
    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // пробуем преобразовать узел в элемент.
        if(!e.isNull()) {
            //qDebug() << e.tagName() << '\n'; // узел действительно является элементом.
            //qDebug() << "attr: " << e.attribute("id") << '\n';
            idModule = e.attribute("id").toInt();
            if (idModule) {
                 // удобнее это вынести в функцию
                for (module = hardwareVector.begin(); module != hardwareVector.end(); ++module) {
                    if ((*module)->idModule == idModule) {
                        (*module)->parseXml(e);
                    }
                }
            }
        }
        n = n.nextSibling();
    }
    lockHardwareVector.unlock();
}
