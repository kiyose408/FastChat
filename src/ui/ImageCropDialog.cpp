#include "ImageCropDialog.h"
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>

ImageCropDialog::ImageCropDialog(const QPixmap& pixmap, QWidget *parent)
    : QDialog(parent)
    , m_originalPixmap(pixmap)
    , m_isDragging(false)
    , m_dragMode(0)
{
    setWindowTitle(QString::fromUtf8("裁切头像"));
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setModal(true);
    setFixedSize(500, 600);
    
    setupUI();
}

ImageCropDialog::~ImageCropDialog()
{
}

void ImageCropDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    QLabel* titleLabel = new QLabel(QString::fromUtf8("拖动选择裁切区域 (1:1)"));
    titleLabel->setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    int maxSize = 400;
    if (m_originalPixmap.width() > maxSize || m_originalPixmap.height() > maxSize) {
        m_scaledPixmap = m_originalPixmap.scaled(maxSize, maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        m_scaledPixmap = m_originalPixmap;
    }
    
    QLabel* cropAreaLabel = new QLabel();
    cropAreaLabel->setFixedSize(m_scaledPixmap.size());
    cropAreaLabel->setPixmap(m_scaledPixmap);
    mainLayout->addWidget(cropAreaLabel, 0, Qt::AlignCenter);
    
    int cropSize = qMin(m_scaledPixmap.width(), m_scaledPixmap.height()) * 0.8;
    m_cropRect = QRect((m_scaledPixmap.width() - cropSize) / 2, 
                       (m_scaledPixmap.height() - cropSize) / 2, 
                       cropSize, cropSize);
    
    QLabel* previewTitle = new QLabel(QString::fromUtf8("预览"));
    previewTitle->setFont(QFont("Microsoft YaHei", 10));
    previewTitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(previewTitle);
    
    m_previewLabel = new QLabel();
    m_previewLabel->setFixedSize(100, 100);
    m_previewLabel->setScaledContents(true);
    m_previewLabel->setStyleSheet("border: 1px solid #CCCCCC; border-radius: 50px;");
    mainLayout->addWidget(m_previewLabel, 0, Qt::AlignCenter);
    
    updatePreview();
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(20);
    
    m_confirmBtn = new QPushButton(QString::fromUtf8("确认"));
    m_confirmBtn->setFixedSize(100, 35);
    m_confirmBtn->setStyleSheet("QPushButton { background-color: #3498DB; color: white; border: none; border-radius: 5px; }"
                                 "QPushButton:hover { background-color: #2980B9; }");
    
    m_cancelBtn = new QPushButton(QString::fromUtf8("取消"));
    m_cancelBtn->setFixedSize(100, 35);
    m_cancelBtn->setStyleSheet("QPushButton { background-color: #E0E0E0; color: #333333; border: none; border-radius: 5px; }"
                                "QPushButton:hover { background-color: #D0D0D0; }");
    
    btnLayout->addStretch();
    btnLayout->addWidget(m_confirmBtn);
    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addStretch();
    
    mainLayout->addLayout(btnLayout);
    
    connect(m_confirmBtn, &QPushButton::clicked, this, &ImageCropDialog::onConfirmClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &ImageCropDialog::onCancelClicked);
}

void ImageCropDialog::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QLabel* cropAreaLabel = nullptr;
    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        QPixmap pm = label->pixmap();
        if (!pm.isNull() && label != m_previewLabel) {
            cropAreaLabel = label;
            break;
        }
    }
    
    if (cropAreaLabel) {
        QRect widgetRect = cropAreaLabel->geometry();
        
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRect(m_cropRect);
        
        painter.setPen(QPen(Qt::white, 1, Qt::DashLine));
        int thirdW = m_cropRect.width() / 3;
        int thirdH = m_cropRect.height() / 3;
        painter.drawLine(m_cropRect.left() + thirdW, m_cropRect.top(), m_cropRect.left() + thirdW, m_cropRect.bottom());
        painter.drawLine(m_cropRect.left() + 2 * thirdW, m_cropRect.top(), m_cropRect.left() + 2 * thirdW, m_cropRect.bottom());
        painter.drawLine(m_cropRect.left(), m_cropRect.top() + thirdH, m_cropRect.right(), m_cropRect.top() + thirdH);
        painter.drawLine(m_cropRect.left(), m_cropRect.top() + 2 * thirdH, m_cropRect.right(), m_cropRect.top() + 2 * thirdH);
        
        QColor overlayColor(0, 0, 0, 120);
        painter.setPen(Qt::NoPen);
        painter.setBrush(overlayColor);
        
        QRect imageArea = cropAreaLabel->geometry();
        painter.drawRect(QRect(imageArea.left(), imageArea.top(), imageArea.width(), m_cropRect.top() - imageArea.top()));
        painter.drawRect(QRect(imageArea.left(), m_cropRect.bottom(), imageArea.width(), imageArea.bottom() - m_cropRect.bottom()));
        painter.drawRect(QRect(imageArea.left(), m_cropRect.top(), m_cropRect.left() - imageArea.left(), m_cropRect.height()));
        painter.drawRect(QRect(m_cropRect.right(), m_cropRect.top(), imageArea.right() - m_cropRect.right(), m_cropRect.height()));
    }
}

void ImageCropDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QLabel* cropAreaLabel = nullptr;
        QList<QLabel*> labels = findChildren<QLabel*>();
        for (QLabel* label : labels) {
            QPixmap pm = label->pixmap();
            if (!pm.isNull() && label != m_previewLabel) {
                cropAreaLabel = label;
                break;
            }
        }
        
        if (cropAreaLabel) {
            QRect imageArea = cropAreaLabel->geometry();
            QPoint pos = event->pos();
            
            if (imageArea.contains(pos)) {
                m_dragMode = getEdge(pos);
                if (m_dragMode > 0 || m_cropRect.contains(pos)) {
                    m_isDragging = true;
                    m_dragStartPos = pos;
                }
            }
        }
    }
    QDialog::mousePressEvent(event);
}

void ImageCropDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging) {
        QPoint pos = event->pos();
        updateCropRect(pos);
        updatePreview();
        update();
    }
    QDialog::mouseMoveEvent(event);
}

void ImageCropDialog::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDragging = false;
    m_dragMode = 0;
    QDialog::mouseReleaseEvent(event);
}

int ImageCropDialog::getEdge(const QPoint& pos) const
{
    int margin = 10;
    
    bool nearLeft = qAbs(pos.x() - m_cropRect.left()) < margin;
    bool nearRight = qAbs(pos.x() - m_cropRect.right()) < margin;
    bool nearTop = qAbs(pos.y() - m_cropRect.top()) < margin;
    bool nearBottom = qAbs(pos.y() - m_cropRect.bottom()) < margin;
    
    if (nearTop && nearLeft) return 1;
    if (nearTop && nearRight) return 2;
    if (nearBottom && nearLeft) return 3;
    if (nearBottom && nearRight) return 4;
    if (nearTop) return 5;
    if (nearBottom) return 6;
    if (nearLeft) return 7;
    if (nearRight) return 8;
    if (m_cropRect.contains(pos)) return 9;
    
    return 0;
}

void ImageCropDialog::updateCropRect(const QPoint& pos)
{
    QLabel* cropAreaLabel = nullptr;
    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        QPixmap pm = label->pixmap();
        if (!pm.isNull() && label != m_previewLabel) {
            cropAreaLabel = label;
            break;
        }
    }
    
    if (!cropAreaLabel) return;
    
    QRect imageArea = cropAreaLabel->geometry();
    QPoint delta = pos - m_dragStartPos;
    
    int minSize = 50;
    
    switch (m_dragMode) {
        case 1: {
            int newSize = m_cropRect.right() - pos.x();
            newSize = qMax(minSize, newSize);
            newSize = qMin(newSize, qMin(imageArea.width(), imageArea.height()));
            m_cropRect.setLeft(m_cropRect.right() - newSize);
            m_cropRect.setTop(m_cropRect.bottom() - newSize);
            break;
        }
        case 2: {
            int newSize = pos.x() - m_cropRect.left();
            newSize = qMax(minSize, newSize);
            m_cropRect.setRight(m_cropRect.left() + newSize);
            m_cropRect.setTop(m_cropRect.bottom() - newSize);
            break;
        }
        case 3: {
            int newSize = m_cropRect.right() - pos.x();
            newSize = qMax(minSize, newSize);
            m_cropRect.setLeft(m_cropRect.right() - newSize);
            m_cropRect.setBottom(m_cropRect.top() + newSize);
            break;
        }
        case 4: {
            int newSize = pos.x() - m_cropRect.left();
            newSize = qMax(minSize, newSize);
            m_cropRect.setRight(m_cropRect.left() + newSize);
            m_cropRect.setBottom(m_cropRect.top() + newSize);
            break;
        }
        case 5: {
            int newTop = m_cropRect.top() + delta.y();
            if (newTop >= imageArea.top() && newTop < m_cropRect.bottom() - minSize) {
                int newHeight = m_cropRect.bottom() - newTop;
                m_cropRect.setTop(newTop);
                m_cropRect.setHeight(newHeight);
                m_cropRect.setWidth(newHeight);
            }
            break;
        }
        case 6: {
            int newBottom = m_cropRect.bottom() + delta.y();
            if (newBottom <= imageArea.bottom() && newBottom > m_cropRect.top() + minSize) {
                int newHeight = newBottom - m_cropRect.top();
                m_cropRect.setHeight(newHeight);
                m_cropRect.setWidth(newHeight);
            }
            break;
        }
        case 7: {
            int newLeft = m_cropRect.left() + delta.x();
            if (newLeft >= imageArea.left() && newLeft < m_cropRect.right() - minSize) {
                int newWidth = m_cropRect.right() - newLeft;
                m_cropRect.setLeft(newLeft);
                m_cropRect.setWidth(newWidth);
                m_cropRect.setHeight(newWidth);
            }
            break;
        }
        case 8: {
            int newRight = m_cropRect.right() + delta.x();
            if (newRight <= imageArea.right() && newRight > m_cropRect.left() + minSize) {
                int newWidth = newRight - m_cropRect.left();
                m_cropRect.setWidth(newWidth);
                m_cropRect.setHeight(newWidth);
            }
            break;
        }
        case 9: {
            QRect newRect = m_cropRect.translated(delta);
            if (newRect.left() >= imageArea.left() && newRect.right() <= imageArea.right() &&
                newRect.top() >= imageArea.top() && newRect.bottom() <= imageArea.bottom()) {
                m_cropRect = newRect;
            }
            break;
        }
    }
    
    m_cropRect = m_cropRect.intersected(imageArea);
    
    if (m_cropRect.width() != m_cropRect.height()) {
        int size = qMin(m_cropRect.width(), m_cropRect.height());
        m_cropRect.setSize(QSize(size, size));
    }
    
    m_dragStartPos = pos;
}

void ImageCropDialog::updatePreview()
{
    double scaleX = (double)m_originalPixmap.width() / m_scaledPixmap.width();
    double scaleY = (double)m_originalPixmap.height() / m_scaledPixmap.height();
    
    QRect originalCropRect(m_cropRect.left() * scaleX, m_cropRect.top() * scaleY,
                           m_cropRect.width() * scaleX, m_cropRect.height() * scaleY);
    
    QPixmap cropped = m_originalPixmap.copy(originalCropRect);
    QPixmap preview = cropped.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    
    QPixmap rounded(100, 100);
    rounded.fill(Qt::transparent);
    
    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, 100, 100);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, preview);
    
    m_previewLabel->setPixmap(rounded);
}

QPixmap ImageCropDialog::getCroppedImage() const
{
    double scaleX = (double)m_originalPixmap.width() / m_scaledPixmap.width();
    double scaleY = (double)m_originalPixmap.height() / m_scaledPixmap.height();
    
    QRect originalCropRect(m_cropRect.left() * scaleX, m_cropRect.top() * scaleY,
                           m_cropRect.width() * scaleX, m_cropRect.height() * scaleY);
    
    return m_originalPixmap.copy(originalCropRect);
}

void ImageCropDialog::onConfirmClicked()
{
    accept();
}

void ImageCropDialog::onCancelClicked()
{
    reject();
}
