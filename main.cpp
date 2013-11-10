#include <QCoreApplication>
#include "corebycetoolthread.h"

/*
 * считываем настройки, делаем список опрашиваемых устройств
 * запускаем поток (наверно все будет в одном потоке) опроса
 * переодически проверяем файл (а тут легче два потока) с сокетами
 * генерим файл с событиями
 */


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    CoreByceToolThread *core = new CoreByceToolThread(&a);

    // This will cause the application to exit when
    // the task signals finished.
    QObject::connect(core, SIGNAL(finish()), &a, SLOT(quit()));

    // This will run the task from the application event loop.
    core->start();

    return a.exec();
}
