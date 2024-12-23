#ifndef EKRAN_H
#define EKRAN_H

#include <QWidget>
#include <QPushButton>
#include <QColor>

#include "ellipsewindow.h"

struct BezierPoint
{
    QPoint point;
    int radius;

    float distanceSquared(const QPoint& other) const
    {
        float dx = point.x() - other.x();
        float dy = point.y() - other.y();

        return dx * dx + dy * dy;
    }
};


struct PixelColor
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t alpha;

    auto operator<=>(const PixelColor& other) const = default;
};



class Ekran : public QWidget
{
    Q_OBJECT
public:
    explicit Ekran(QWidget *parent = nullptr);

    ~Ekran();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void drawPixel(QImage& img, int x, int y, int h_color);
    void drawPixel(QImage& img, const QPoint& point, const PixelColor& color);
    void drawLine(QImage& img, const QPoint& frist, const QPoint& second);
    void drawLineBresenham(QImage& img, const QPoint& first, const QPoint& second);
    void drawCircle(QImage& img, const QPoint& first, const QPoint& second);
    void drawCircle(QImage& img, const QPoint& first, float r);
    void drawEllipse(QImage& img, const QPoint& first, const QPoint& second, int N);
    void drawBezierCurve(QImage& img, const std::vector<BezierPoint>& controlPoints, int N);

    void flood_fill(QImage& img, const QPoint& point, const PixelColor& currentColor, const PixelColor& newColor);
    void scanLineFill(QImage& img, const std::vector<QPoint>& points);

    PixelColor getPixelColor(const QImage& img, const QPoint& point) const;


    void clear();
    void removePoints(std::vector<BezierPoint>& bezierPoints, const QPoint& position);
    void Copy(QImage& image, QImage& image2);


    enum class drawingMode
    {
        Line, Circle, Ellipse, BezierCurve, FillWithColor, ScanLineFillMode
    };


private:
    QImage m_canvas;
    QImage m_tempImage;
    QPoint m_startPoint;
    QPoint m_endPoint;
    bool m_isDrawing;
    int m_ellipseN;
    int m_draggedPoint;
    bool m_isDraggedPoint;

    std::vector<BezierPoint> m_BezierCurvePoints;
    std::vector<QPoint> m_points;

    drawingMode m_mode;
    QPushButton* m_lineButton;
    QPushButton* m_CircleButton;
    QPushButton* m_EllipseButton;
    QPushButton* m_ClearButton;
    QPushButton* m_BezierCurveButton;
    QPushButton* m_FillColorButton;
    QPushButton* m_ScanLineFillButton;

    EllipseWindow* m_ellipseWindow;

private slots:
    void setLineMode();
    void setCircleMode();
    void setEllipseMode();
    void setBezierCurveMode();
    void setFillWithColorMode();
    void setScanLineFillMode();
    void clearAll();
    void updateEllipseN(int value);

signals:
};

#endif // EKRAN_H
