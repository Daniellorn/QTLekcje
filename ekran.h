#ifndef EKRAN_H
#define EKRAN_H

#include <QWidget>

class Ekran : public QWidget
{
    Q_OBJECT
public:
    explicit Ekran(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void drawPixel(QImage& img, int x, int y, int h_color);
    void drawLine(QImage& img, const QPoint& frist, const QPoint& second);
    void drawLineBresenham(QImage& img, const QPoint& first, const QPoint& second);
    void drawCircle(QImage& img, const QPoint& first, const QPoint& second);

private:
    QImage m_canvas;
    QPoint m_startPoint;
    QPoint m_endPoint;
    bool m_isDrawing;

signals:
};

#endif // EKRAN_H
