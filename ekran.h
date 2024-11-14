#ifndef EKRAN_H
#define EKRAN_H

#include <QWidget>
#include <QPushButton>

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
    void drawEllipse(QImage& img, const QPoint& first, const QPoint& second, int N);

    enum class drawingMode
    {
        Line, Circle, Ellipse
    };


private:
    QImage m_canvas;
    QPoint m_startPoint;
    QPoint m_endPoint;
    bool m_isDrawing;


    drawingMode m_mode;
    QPushButton* m_lineButton;
    QPushButton* m_CircleButton;
    QPushButton* m_EllipseButton;
    QPushButton* m_ClearButton;

private slots:
    void setLineMode();
    void setCircleMode();
    void setEllipseMode();
    void clear();

signals:
};

#endif // EKRAN_H
