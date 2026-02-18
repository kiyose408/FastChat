#ifndef MESSAGEDELEGATE_H
#define MESSAGEDELEGATE_H

#include <QStyledItemDelegate>
#include <QNetworkAccessManager>
#include <QCache>

class MessageDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    void imageClicked(const QString& fileUrl);
    void fileClicked(const QString& fileUrl, const QString& fileName);

private:
    QSize calculateTextSize(const QString &text) const;
    void paintTextMessage(QPainter *painter, const QStyleOptionViewItem &option, bool isSelf, const QString &text, const QString &time, bool isRead) const;
    void paintImageMessage(QPainter *painter, const QStyleOptionViewItem &option, bool isSelf, const QString &fileUrl, const QString &time, bool isRead) const;
    void paintFileMessage(QPainter *painter, const QStyleOptionViewItem &option, bool isSelf, const QString &fileName, const QString &fileUrl, const QString &time, bool isRead) const;
    
    mutable QNetworkAccessManager m_networkManager;
    mutable QCache<QString, QPixmap> m_imageCache;
    mutable QMap<QString, bool> m_loadingImages;
    
    void loadImage(const QString& url) const;
    mutable QRect m_lastImageRect;
    mutable QString m_lastImageFileUrl;
    mutable QRect m_lastFileRect;
    mutable QString m_lastFileUrl;
    mutable QString m_lastFileName;
};

#endif // MESSAGEDELEGATE_H
