#ifndef HARDWARE_H
#define HARDWARE_H

#include <QObject>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>

#include "module.h"
#include "modbus.h"


/*
 * базовый класс аппаратного девайса
 * уже добавлено понятие модбаса, адреса устройства
 * методы для работы с устройством
 * все аппаратные штуки должны наследоваться от него
 */

class Hardware : public QObject, public Module
{
    Q_OBJECT

public:
    Hardware(QObject *parent = 0, QString n = "HardwareModule");

    QString name;

    static void setMbPort(modbus_t *port);

    void setMbAddr(int addr);
    int getMbAddr();
    quint16 readReg(int regAddr);
    void writeReg(int regAddr, quint16 data);
    void settings();

    virtual bool refresh() = 0;
    virtual bool isEvent() = 0;
    virtual void generateXml(QTextStream &out);
    virtual void parseXml(QDomElement &domElement) = 0;

protected:
    static modbus_t *mbPort_;
    int readRegisters(int regAddr, int cont, quint16* data);

    int mbAddr_;
    bool refreshEvent_;         // событие для обновления данных с девайса
    bool isFirstRefresh_;       // признак первого обмена

private:




};

#endif // HARDWARE_H
