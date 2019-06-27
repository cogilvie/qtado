#include "tado.h"

#include <QDebug>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QUrlQuery>
#include <QtNetwork/QNetworkSession>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QHttpMultiPart>

#define TADO_AUTH_URL QStringLiteral("https://auth.tado.com/oauth/token")
#define TADO_ME_URL QStringLiteral("https://my.tado.com/api/v2/me")
#define TADO_WEATHER_URL QStringLiteral("https://my.tado.com/api/v2/homes/%1/weather")
#define TADO_ZONES_URL QStringLiteral("https://my.tado.com/api/v2/homes/%1/zones")
#define TADO_ZONE_INFO_URL QStringLiteral("https://my.tado.com/api/v2/homes/%1/zones/%2/state")

#define TADO_CLIENT_ID  QStringLiteral("tado-web-app")
#define TADO_CLIENT_SECRET  QStringLiteral("wZaRN7rpjn3FoNyF5IFuxg9uMzYJcvOoQ8QWiIqS3hfk6gLhVlG57j5YNoZL2Rtc")
#define TADO_SCOPE QStringLiteral("home.user")

#define TADO_HOMEID 116061

Tado::Tado(const QString &username, const QString &password, QObject *parent)
    : QObject(parent)
    , m_username(username)
    , m_password(password)
    , m_manager(new QNetworkAccessManager())
{
    m_authenticateTimer.setSingleShot(true);

}

bool Tado::isAuthenticated()
{
    return m_authenticateTimer.isActive();
}

void Tado::authenticate()
{
    m_authticating = true;
    QUrl url(TADO_AUTH_URL);
    QUrlQuery query;
    query.addQueryItem("client_id", TADO_CLIENT_ID);
    query.addQueryItem("client_secret", TADO_CLIENT_SECRET);
    query.addQueryItem("grant_type", "password");
    query.addQueryItem("scope", TADO_SCOPE);
    query.addQueryItem("username", m_username);
    query.addQueryItem("password", m_password);
    url.setQuery(query);

    QNetworkRequest tokenRequest(url);
    tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply * reply = m_manager->post(tokenRequest, QByteArray());
    connect(reply, &QNetworkReply::finished, this, &Tado::handleTokenResponse);
}

void Tado::requestHomeId()
{
    disconnect(this, &Tado::authenticated, this, nullptr);
    if (!isAuthenticated()) {
        connect(this, &Tado::authenticated, this, &Tado::requestHomeId);
        if(!m_authticating) {
            authenticate();
        }
    } else {
        const QNetworkReply * reply = m_manager->get(getRequest(TADO_ME_URL));
        connect(reply, &QNetworkReply::finished, this, &Tado::handelMeResponse);
    }
}

void Tado::requestZoneList(const int homeId)
{
    disconnect(this, &Tado::authenticated, this, nullptr);
    if (!isAuthenticated()) {
        connect(this, &Tado::authenticated, this, [this, homeId] () {
            requestZoneList(homeId);
        });
        if(!m_authticating) {
            authenticate();
        }
    } else {
        const QNetworkReply * reply = m_manager->get(getRequest(QString(TADO_ZONES_URL).arg(homeId)));
        connect(reply, &QNetworkReply::finished, this, &Tado::handelZoneListResponse);
    }
}

void Tado::requestZoneStatus(const int homeId, const int zoneId)
{
    disconnect(this, &Tado::authenticated, this, nullptr);
    if (!isAuthenticated()) {
        connect(this, &Tado::authenticated, this, [this, homeId, zoneId] () {
            requestZoneStatus(homeId, zoneId);
        });
        if(!m_authticating) {
            authenticate();
        }
    } else {
        const QNetworkReply * reply = m_manager->get(getRequest(QString(TADO_ZONE_INFO_URL).arg(homeId).arg(zoneId)));
        connect(reply, &QNetworkReply::finished, this, &Tado::handelZoneStatusResponse);
    }
}

void Tado::handleTokenResponse()
{
    QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(sender());
    if (networkReply) {
        if (!networkReply->error()) {

            const QJsonDocument document = QJsonDocument::fromJson(networkReply->readAll());
            const QJsonObject object = document.object();

            if (object.contains("access_token")) {
                m_token = object.value("access_token").toString();

                //Schedure a rerequest just before it should expire
                m_authenticateTimer.start((object.value("expires_in").toInt(599) - 1)*1000);
                m_authticating = false;
                emit authenticated();
            } else {
                 authenticate();
            }
        } else {
            qWarning() << "auth error" << networkReply->errorString();
            authenticate();
        }

        networkReply->deleteLater();
    }
}

void Tado::handelZoneStatusResponse()
{
    QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(sender());
    if (networkReply) {
        if (!networkReply->error()) {

            //A little hack but saves finding a way to pass it into the slot
            const QStringList query =  networkReply->url().toString().split('/');
            const int id = query.at(query.length()-2).toInt();

            const QJsonDocument document = QJsonDocument::fromJson(networkReply->readAll());
            const QJsonObject object = document.object();

            const QJsonObject sensorobject = object.value(QStringLiteral("sensorDataPoints")).toObject();
            const double currentTemperature = sensorobject.value(QStringLiteral("insideTemperature")).toObject().value(QStringLiteral("celsius")).toDouble();
            const double currentHumidity = sensorobject.value(QStringLiteral("humidity")).toObject().value(QStringLiteral("percentage")).toDouble();

            const QJsonObject settingObject = object.value(QStringLiteral("setting")).toObject();
            const bool active = settingObject.value(QStringLiteral("power")).toString() == QStringLiteral("ON");
            const double targetTemperature = settingObject.value(QStringLiteral("temperature")).toObject().value(QStringLiteral("celsius")).toDouble();

            const QJsonObject activityobject = object.value(QStringLiteral("activityDataPoints")).toObject();
            const double heatingPower = activityobject.value(QStringLiteral("heatingPower")).toObject().value(QStringLiteral("percentage")).toDouble();

            const ZoneStatus status {id, currentTemperature, currentHumidity, targetTemperature, active, heatingPower};

            emit zoneStatusRecieved(status);
        } else {
            qWarning() << "z error" << networkReply->errorString();
        }

        networkReply->deleteLater();
    }
}

void Tado::handelZoneListResponse()
{
    QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(sender());
    if (networkReply) {
        if (!networkReply->error()) {

            const QJsonDocument document = QJsonDocument::fromJson(networkReply->readAll());
            const QJsonArray array = document.array();
            QList<ZoneInfo> zones;
            for(const QJsonValue v : array) {
                const QJsonObject object = v.toObject();
                const ZoneInfo z {object.value(QStringLiteral("id")).toInt(), object.value(QStringLiteral("name")).toString()};
                zones.append(z);
            }
            emit zoneListRecieved(zones);
        } else {
            qWarning() << "zone error" << networkReply->errorString();
        }

        networkReply->deleteLater();
    }
}

void Tado::handelMeResponse()
{
    QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(sender());
    if (networkReply) {
        if (!networkReply->error()) {

            const QJsonDocument document = QJsonDocument::fromJson(networkReply->readAll());
            const QJsonObject object = document.object();
            if (object.contains("homes")) {
                const QJsonArray homes = object.value("homes").toArray();
                if (!homes.isEmpty()) {
                    const int homeId = homes.at(0).toObject().value("id").toInt();
                    emit homeIdRecieved(homeId);
                }
            }
        } else {
            qWarning() << "me error" << networkReply->errorString();
        }

        networkReply->deleteLater();
    }
}

QNetworkRequest Tado::getRequest(const QString &url)
{
    const QUrl requestUrl(url);
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Authorization", QByteArray("Bearer ").append(m_token));
    return request;
}

QDebug operator<<(QDebug debug, const Tado::ZoneInfo &zi)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "ZoneInfo(Id: " << zi.id << ", Name: " << zi.name << ')';

    return debug;
}

QDebug operator<<(QDebug debug, const Tado::ZoneStatus &zs)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "ZoneStatus(Id: " << zs.id << ", Temperature: " << zs.temperature << ", Humidity: " << zs.humidity <<
                       ", Target: " << zs.targetTemperature <<", Power: " << zs.power << " " << zs.heatingPower <<"% )";

    return debug;
}
