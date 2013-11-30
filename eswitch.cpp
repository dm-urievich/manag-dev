#include "eswitch.h"

eSwitch::eSwitch(QObject *parent, QString name, int addr) : Hardware(parent, name)
{

    m_mbAddr = addr;

    eSwitchOutState = false;
    dInState = false;
    adcData = 0;
    m_raiseEvent = false;
    m_fallEvent = false;
    m_adcEvent = false;
    adcHysteresis = 5;
}

bool eSwitch::refresh(void)
{
    bool prevState = dInState;
    bool prevOutState = eSwitchOutState;
    int regAddr = 0;
    quint16 data[6];

    //data = readReg(regAddr);
    if (readRegisters(regAddr, 7, data) == -1) {
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
        m_fallEvent = true;
    }

    if (dInState && !prevState) { // передний фронт
        emit dInRaise();
        m_raiseEvent = true;
    }

    if (!eSwitchOutState && prevOutState)
        m_offEvent = true;

    if (eSwitchOutState && !prevOutState)
        m_onEvent = true;

    // заполняем данные с АЦП
    if (data[5] >= adcData + adcHysteresis) {
        m_adcEvent = true;
        adcData = data[5] - (data[5] % adcHysteresis);
    }
    else
        if (data[5] <= adcData - adcHysteresis) {
            m_adcEvent = true;
            adcData = data[5] - (data[5] % adcHysteresis);
        }
    //adcData = data[5];

    if (data[6] == 1) {
        ledState = true;
    }
    else {
        ledState = false;
    }

    // обменялись впервые после создания класса, нужно обновить статические
    // данные с девайса
    if (m_isFirstRefresh) {
        m_refreshEvent = true;
        m_isFirstRefresh = false;
    }

    return true;
}

bool eSwitch::isEvent()
{
    return (m_raiseEvent || m_fallEvent || m_adcEvent || m_refreshEvent || m_offEvent || m_onEvent);
}

void eSwitch::generateXml(QTextStream &out)
{
    out << "<modbusSwitch id=\"" << idModule <<"\">\n";
    out << "<idModule>"     << idModule         << "</idModule>\n";
    out << "<name>"         << name             << "</name>\n";
    out << "<mbAddr>"       << m_mbAddr         << "</mbAddr>\n";
    out << "<outState>"     << eSwitchOutState  << "</outState>\n";
    out << "<dinState>"     << dInState         << "</dinState>\n";
    out << "<adcData>"      << adcData          << "</adcData>\n";
    out << "<ledState>"     << ledState         << "</ledState>\n";
    out << "<adcEvent>"     << m_adcEvent       << "</adcEvent>\n";
    out << "<raiseEvent>"   << m_raiseEvent     << "</raiseEvent>\n";
    out << "<fallEvent>"    << m_fallEvent      << "</fallEvent>\n";
    out << "</modbusSwitch>\n";

    m_raiseEvent = false;
    m_fallEvent = false;
    m_adcEvent = false;
    m_refreshEvent = false;
    m_offEvent = false;
    m_onEvent = false;

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

void eSwitch::ledOn()
{
    int regAddr = 6;
    int data = 1;

    writeReg(regAddr, data);
}

void eSwitch::ledOff()
{
    int regAddr = 6;
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
            if (tagName == "ledOnSocket") {
                if (e.text().toInt()) {
                    ledOn();
                }
            }
            if (tagName == "ledOffSocket") {
                if (e.text().toInt()) {
                    ledOff();
                }
            }
        }
        n = n.nextSibling();
    }
}
