#include "updatechecker.h"
#include "constants.h"
#include "http.h"

UpdateChecker::UpdateChecker() {
    m_needUpdate = false;
}

void UpdateChecker::checkForUpdate() {
    QUrl url(QString(Constants::WEBSITE) + "-ws/release.xml");

    QUrlQuery q;
    q.addQueryItem("v", Constants::VERSION);

#ifdef APP_MAC
    q.addQueryItem("os", "mac");
#endif
#ifdef APP_WIN
    q.addQueryItem("os", "win");
#endif
#ifdef APP_MAC_STORE
    q.addQueryItem("store", "mac");
#endif
    url.setQuery(q);

    QObject *reply = Http::instance().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(requestFinished(QByteArray)));
}

void UpdateChecker::requestFinished(const QByteArray &data) {
    UpdateCheckerStreamReader reader;
    reader.read(data);
    m_needUpdate = reader.needUpdate();
    m_remoteVersion = reader.remoteVersion();
    if (m_needUpdate && !m_remoteVersion.isEmpty()) emit newVersion(m_remoteVersion);
}

QString UpdateChecker::remoteVersion() {
    return m_remoteVersion;
}

// --- Reader ---

bool UpdateCheckerStreamReader::read(const QByteArray &data) {
    addData(data);

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == "release") {
                while (!atEnd()) {
                    readNext();
                    if (isStartElement() && name() == "version") {
                        QString remoteVersion = readElementText();
                        qDebug() << remoteVersion << QString(Constants::VERSION);
                        m_needUpdate = remoteVersion != QString(Constants::VERSION);
                        m_remoteVersion = remoteVersion;
                        break;
                    }
                }
            }
        }
    }

    return !error();
}

QString UpdateCheckerStreamReader::remoteVersion() {
    return m_remoteVersion;
}
