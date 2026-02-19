#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QObject>
#include <QPixmap>
#include <QString>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <functional>

class ImageLoader : public QObject
{
    Q_OBJECT

public:
    static ImageLoader& instance()
    {
        static ImageLoader loader;
        return loader;
    }
    
    void load(const QString& url, std::function<void(QPixmap)> callback)
    {
        if (url.isEmpty()) {
            return;
        }
        
        if (m_cache.contains(url)) {
            callback(m_cache[url]);
            return;
        }
        
        if (m_loading.contains(url)) {
            return;
        }
        
        m_loading[url] = true;
        
        QString fullUrl = url.startsWith("http") ? url : ("http://localhost:3000" + url);
        QNetworkReply* reply = m_manager.get(QNetworkRequest(QUrl(fullUrl)));
        
        connect(reply, &QNetworkReply::finished, this, [this, reply, url, callback]() {
            m_loading.remove(url);
            
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(data)) {
                    m_cache[url] = pixmap;
                    callback(pixmap);
                }
            }
            reply->deleteLater();
        });
    }
    
    void clearCache()
    {
        m_cache.clear();
    }
    
    bool isCached(const QString& url) const
    {
        return m_cache.contains(url);
    }
    
    QPixmap getCached(const QString& url) const
    {
        return m_cache.value(url);
    }

private:
    ImageLoader() {}
    ~ImageLoader() {}
    
    QNetworkAccessManager m_manager;
    QMap<QString, QPixmap> m_cache;
    QMap<QString, bool> m_loading;
};

#endif // IMAGELOADER_H
