#include "module.h"

Module::Module()
{
}

void Module::settings(void)
{

    qDebug() << "i'm virtual in base class";
}

bool Module::isEvent()
{
    return false;
}
