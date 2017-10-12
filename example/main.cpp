#include <QGuiApplication>

#include <tado.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Tado* t = new Tado(TADO_USER, TADO_PASSWORD);
    int homeId;
    QObject::connect(t, &Tado::homeIdRecieved, [t, &homeId](int id){
        qDebug() << "Home Id:" << id;
        homeId = id;
        t->requestZoneList(homeId);
    });

    QObject::connect(t, &Tado::zoneListRecieved, [t, &homeId](const QList<Tado::ZoneInfo>& zones){
        qDebug() << "Zones:";
        for (const Tado::ZoneInfo zone : zones) {
            qDebug() << zone;
            t->requestZoneStatus(homeId, zone.id);
        }
        qDebug() << "Status:";

    });

    QObject::connect(t, &Tado::zoneStatusRecieved, [](const Tado::ZoneStatus& zone) {
        qDebug() << zone;
    });

    t->requestHomeId();
    return a.exec();
}
