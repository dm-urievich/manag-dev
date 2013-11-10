#ifndef TRANSFERTHREAD_H
#define TRANSFERTHREAD_H

#include <QThread>
#include <QDebug>
#include <QVector>
#include <QTime>
#include <QTextStream>
#include <QMutex>

#include "hardware.h"

class TransferThread : public QThread
{
    Q_OBJECT
public:
    explicit TransferThread(QVector<Hardware*> *hardwareVector, QMutex* lockHardwareVector, QObject *parent = 0);


protected:
    void run();

private:
    volatile unsigned int period_;
    QVector<Hardware*> *hardwareVector_;

    QMutex* lockHardwareVector_;
    
signals:
    void transferTime(int time);
    void eventInModule(bool);

public slots:
    void setPeriod(int data);
    
};

#endif // TRANSFERTHREAD_H
