#ifndef TADO_H
#define TADO_H

#include <QObject>
#include <QTimer>
#include <QDebugStateSaver>

#include "libtado.h"

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

class QTADO_EXPORT Tado : public QObject
{
    Q_OBJECT
public:
    explicit Tado(const QString& username, const QString& password, QObject *parent = nullptr);

    bool isAuthenticated();

    struct ZoneInfo
    {
        ZoneInfo(int id, const QString& name)
            : id(id)
            , name(name)
        {}

        int id;
        QString name;
    };

    struct ZoneStatus
    {
        ZoneStatus(int id, const double temperature, const double humidity,
                   const double targetTemperature, const bool power, const double heatingPower)
            : id(id)
            , temperature(temperature)
            , humidity(humidity)
            , targetTemperature(targetTemperature)
            , power(power)
            , heatingPower(heatingPower)
        {}

        int id;
        double temperature;
        double humidity;
        double targetTemperature;
        bool power;
        double heatingPower;

    };

signals:
    void homeIdRecieved(const int id);
    void authenticated();
    void zoneListRecieved(const QList<ZoneInfo>& zones);
    void zoneStatusRecieved(const ZoneStatus& status);

public slots:
    void requestHomeId();
    void requestZoneList(const int homeId);
    void requestZoneStatus(const int homeId, const int zoneId);


private slots:
    void authenticate();
    void handleTokenResponse();
    void handelMeResponse();
    void handelZoneListResponse();
    void handelZoneStatusResponse();


private:
    QNetworkRequest getRequest(const QString& url);

private:
    QString m_username;
    QString m_password;

    QString m_token;
    bool m_authticating = false;

    QNetworkAccessManager* m_manager  = nullptr;
    QTimer m_authenticateTimer;

};

QDebug operator<<(QDebug debug, const Tado::ZoneInfo &zi);

QDebug operator<<(QDebug debug, const Tado::ZoneStatus &zs);

#endif // TADO_H
