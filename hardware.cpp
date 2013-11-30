#include "hardware.h"

Hardware::Hardware(QObject *parent, QString n) :
      QObject(parent),
      Module(),
      name(n)
{
    //mbPort_ = 0;
    m_refreshEvent = false;
    m_isFirstRefresh = true;
}

modbus_t* Hardware::m_mbPort = 0;

void Hardware::setMbPort(modbus_t *port)
{
    m_mbPort = port;
}

void Hardware::setMbAddr(int addr)
{
    m_mbAddr = addr;
}

int Hardware::getMbAddr(void)
{
    return m_mbAddr;
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
    if (m_mbPort != 0) {
        modbus_set_slave(m_mbPort, m_mbAddr);
        return modbus_read_registers(m_mbPort, regAddr, cont, data);
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
    if (m_mbPort != 0) {
        modbus_set_slave(m_mbPort, m_mbAddr);
        modbus_write_register(m_mbPort, regAddr, data);
    }
}

void Hardware::generateXml(QTextStream &out)
{
    QDomDocument hardware;
    QDomElement root = hardware.createElement("hardware");
    QDomElement addr = hardware.createElement("mbAddr");
    QDomElement nameDev = hardware.createElement("name");
    QDomText nameModule = hardware.createTextNode(name);
    QDomText addrModule = hardware.createTextNode(QString::number(m_mbAddr));

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

