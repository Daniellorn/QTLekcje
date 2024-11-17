#ifndef EKRAN_H
#define EKRAN_H

#include <QWidget>
#include <QPushButton>

#include "ellipsewindow.h"

struct BezierPoint
{
    QPoint point;
    int radius;
};



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
    void drawCircle(QImage& img, const QPoint& first, float r);
    void drawEllipse(QImage& img, const QPoint& first, const QPoint& second, int N);

    enum class drawingMode
    {
        Line, Circle, Ellipse, BezierCurve
    };


    void clear();


private:
    QImage m_canvas;
    QPoint m_startPoint;
    QPoint m_endPoint;
    bool m_isDrawing;
    int m_ellipseN;

    std::vector<BezierPoint> m_BezierCurvePoints;

    drawingMode m_mode;
    QPushButton* m_lineButton;
    QPushButton* m_CircleButton;
    QPushButton* m_EllipseButton;
    QPushButton* m_ClearButton;
    QPushButton* m_BezierCurve;

    EllipseWindow* m_ellipseWindow;

private slots:
    void setLineMode();
    void setCircleMode();
    void setEllipseMode();
    void setBezierCurveMode();
    void clearAll();
    void updateEllipseN(int value);

signals:
};

#endif // EKRAN_H
