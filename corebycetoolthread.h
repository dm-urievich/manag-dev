#ifndef COREBYCETOOLTHREAD_H
#define COREBYCETOOLTHREAD_H

#include <QSettings>
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QDomDocument>
#include <QTimer>
#include <QTextStream>

#include "hardware.h"
#include "eswitch.h"
#include "transferthread.h"


class CoreByceToolThread : public QThread
{
    Q_OBJECT
public:
    explicit CoreByceToolThread(QObject *parent = 0);
    void portOpenClose(void);

    modbus_t *mbPort;
    
signals:
    void guiRefresh();
    void setTransferPeriod(int);
    void finish();

public slots:
    void createNewHardwareModule(Hardware *device);
    void generateXmlHardware(bool isEvent);
    void parseSockets(void);

protected:
    void run();

private:
    void findDevices(QString fileName);

    QTimer* m_readSocketsTimer;

    QSettings *portSettings;

    TransferThread* transferHardwareModules;

    QVector<Hardware*> hardwareVector;

    QMutex lockHardwareVector;

};

#endif // COREBYCETOOLTHREAD_H
