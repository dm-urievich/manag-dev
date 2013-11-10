#include "eswitch.h"

eSwitch::eSwitch(QObject *parent, QString name, int addr) : Hardware(parent, name)
{

    mbAddr_ = addr;

    eSwitchOutState = false;
    dInState = false;
    adcData = 0;
    raiseEvent_ = false;
    fallEvent_ = false;
    adcEvent_ = false;
    adcHysteresis = 5;
}

bool eSwitch::refresh(void)
{
    bool prevState = dInState;
    bool prevOutState = eSwitchOutState;
    int regAddr = 0;
    quint16 data[6];

    //data = readReg(regAddr);
    if (readRegisters(regAddr, 6, data) == -1) {
        return false;
    }

    if (data[0] == 1) {
        eSwitchOutState = true;
    }
    else {
        eSwitchOutState = false;
    }

    if (data[2] == 1)
        dInState = true;
    else
        dInState = false;

    if (!dInState && prevState) {     // задний фронт
        emit dInFall();
        fallEvent_ = true;
    }

    if (dInState && !prevState) { // передний фронт
        emit dInRaise();
        raiseEvent_ = true;
    }

    if (!eSwitchOutState && prevOutState)
        offEvent_ = true;

    if (eSwitchOutState && !prevOutState)
        onEvent_ = true;

    // заполняем данные с АЦП
    if (data[5] >= adcData + adcHysteresis) {
        adcEvent_ = true;
        adcData = data[5] - (data[5] % adcHysteresis);
    }
    else
        if (data[5] <= adcData - adcHysteresis) {
            adcEvent_ = true;
            adcData = data[5] - (data[5] % adcHysteresis);
        }
    //adcData = data[5];

    // обменялись впервые после создания класса, нужно обновить статические
    // данные с девайса
    if (isFirstRefresh_) {
        refreshEvent_ = true;
        isFirstRefresh_ = false;
    }

    return true;
}

bool eSwitch::isEvent()
{
    return (raiseEvent_ || fallEvent_ || adcEvent_ || refreshEvent_ || offEvent_ || onEvent_);
}

void eSwitch::generateXml(QTextStream &out)
{
    out << "<modbusSwitch id=\"" << idModule <<"\">\n";
    out << "<idModule>"     << idModule         << "</idModule>\n";
    out << "<name>"         << name             << "</name>\n";
    out << "<mbAddr>"       << mbAddr_          << "</mbAddr>\n";
    out << "<outState>"     << eSwitchOutState  << "</outState>\n";
    out << "<dinState>"     << dInState         << "</dinState>\n";
    out << "<adcData>"      << adcData          << "</adcData>\n";
    out << "<raiseEvent>"   << raiseEvent_      << "</raiseEvent>\n";
    out << "<fallEvent>"    << fallEvent_       << "</fallEvent>\n";
    out << "</modbusSwitch>\n";

    raiseEvent_ = false;
    fallEvent_ = false;
    adcEvent_ = false;
    refreshEvent_ = false;
    offEvent_ = false;
    onEvent_ = false;

}

void eSwitch::on()
{
    int regAddr = 0;
    int data = 1;

    writeReg(regAddr, data);
}

void eSwitch::off()
{
    int regAddr = 0;
    int data = 0;

    writeReg(regAddr, data);
}

void eSwitch::parseXml(QDomElement &domElement)
{
    QString tagName;

    QDomNode n = domElement.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // пробуем преобразовать узел в элемент.
        if(!e.isNull()) {
            tagName = e.tagName(); // узел действительно является элементом.
            if (tagName == "onSocket") {
                if (e.text().toInt()) {
                    on();
                }
            }
            if (tagName == "offSocket") {
                if (e.text().toInt()) {
                    off();
                }
            }
        }
        n = n.nextSibling();
    }
}
