#include "hardware.h"

Hardware::Hardware(QObject *parent, QString n) :
      QObject(parent),
      Module(),
      name(n)
{
    //mbPort_ = 0;
    refreshEvent_ = false;
    isFirstRefresh_ = true;
}

modbus_t* Hardware::mbPort_ = 0;

void Hardware::setMbPort(modbus_t *port)
{
    mbPort_ = port;
}

void Hardware::setMbAddr(int addr)
{
    mbAddr_ = addr;
}

int Hardware::getMbAddr(void)
{
    return mbAddr_;
}

// чтение одного регистра
quint16 Hardware::readReg(int regAddr)
{
    quint16 data;

    readRegisters(regAddr, 1, &data);

    return data;
}


// чтение группы регистров
int Hardware::readRegisters(int regAddr, int cont, quint16 *data)
{
    if (mbPort_ != 0) {
        modbus_set_slave(mbPort_, mbAddr_);
        return modbus_read_registers(mbPort_, regAddr, cont, data);
    }
    else {
        for (int i = 0; i < cont; i++) {
            data[i] = 0;
        }
    }
    return -1;
}

// запись одного регистра
void Hardware::writeReg(int regAddr, quint16 data)
{
    if (mbPort_ != 0) {
        modbus_set_slave(mbPort_, mbAddr_);
        modbus_write_register(mbPort_, regAddr, data);
    }
}

void Hardware::generateXml(QTextStream &out)
{
    QDomDocument hardware;
    QDomElement root = hardware.createElement("hardware");
    QDomElement addr = hardware.createElement("mbAddr");
    QDomElement nameDev = hardware.createElement("name");
    QDomText nameModule = hardware.createTextNode(name);
    QDomText addrModule = hardware.createTextNode(QString::number(mbAddr_));

    hardware.appendChild(root);
    root.appendChild(addr);
    root.appendChild(nameDev);
    addr.appendChild(addrModule);
    nameDev.appendChild(nameModule);

    hardware.save(out, 4);
}

void Hardware::settings()
{
    qDebug() << "I am Hardware module";
}

