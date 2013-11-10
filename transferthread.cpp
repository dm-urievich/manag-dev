#include "transferthread.h"

TransferThread::TransferThread(QVector<Hardware *> *hardwareVector, QMutex* lockHardwareVector, QObject *parent) :
    QThread(parent)
{
    hardwareVector_ = hardwareVector;
    lockHardwareVector_ = lockHardwareVector;

    period_ = 1000;     // по умолчанию будет 1 с
}

void TransferThread::run()
{
    QVector<Hardware*>::Iterator device;// = hardwareVector_->begin();
    QTime timer;
    timer.start();
    //QTextStream *outXml;
    bool isEvent = false;

    for (;;) {
        timer.restart();        // или за мютексом?
        lockHardwareVector_->lock();
        for (device = hardwareVector_->begin(); device != hardwareVector_->end(); ++device) {
            if ((*device)->refresh()) {
                if ((*device)->isEvent()) {
                    isEvent = true;
                    qDebug() << "Event in " << (*device)->name;
                    /*
                    QFile file((*device)->name + ".xml");
                    if (file.open(QIODevice::WriteOnly)) {
                        outXml = new QTextStream(&file);
                        (*device)->generateXml(*outXml);
                        file.close();
                        delete outXml;
                    }
                    */
                }
            }
            else {
                qDebug() << "No response " << (*device)->name;
            }
            msleep(2);  // время на раздупление (скорее аппаратной части)
        }
        lockHardwareVector_->unlock();

        emit transferTime(timer.elapsed());

        if (isEvent) {
            emit eventInModule(true);
            isEvent = false;
        }

        // для честного опроса нужно от периода вычитать время потраченное на выполнение
        msleep(period_);        // нужно в это время парсить xml с сокетами
    }
}

void TransferThread::setPeriod(int data)
{
    period_ = data;
}
