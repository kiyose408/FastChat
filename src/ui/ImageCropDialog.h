#ifndef IMAGECROPDIALOG_H
#define IMAGECROPDIALOG_H

#include <QDialog>
#include <QPixmap>
#include <QPoint>
#include <QRect>

class QLabel;
class QPushButton;

class ImageCropDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageCropDialog(const QPixmap& pixmap, QWidget *parent = nullptr);
    ~ImageCropDialog();
    
    QPixmap getCroppedImage() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onConfirmClicked();
    void onCancelClicked();

private:
    QPixmap m_originalPixmap;
    QPixmap m_scaledPixmap;
    QRect m_cropRect;
    bool m_isDragging;
    QPoint m_dragStartPos;
    int m_dragMode;
    
    QLabel* m_previewLabel;
    QPushButton* m_confirmBtn;
    QPushButton* m_cancelBtn;
    
    void setupUI();
    void updateCropRect(const QPoint& pos);
    void updatePreview();
    int getEdge(const QPoint& pos) const;
};

#endif // IMAGECROPDIALOG_H
