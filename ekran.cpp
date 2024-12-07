#include "ekran.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QImage>
#include <QColorDialog>

#include <cmath>
#include <vector>
#include <algorithm>
#include <stack>

Ekran::Ekran(QWidget *parent)
    : QWidget{parent}, m_isDrawing(false), m_ellipseN(500), m_draggedPoint(-1), m_isDraggedPoint(false)
{
    m_canvas = QImage(500, 500, QImage::Format_RGB32);
    m_tempImage = QImage(500,500, QImage::Format_RGB32);
    m_canvas.fill(0);

    setFixedSize(700,500);

    m_lineButton = new QPushButton("Line", this);
    m_CircleButton = new QPushButton("Circle", this);
    m_EllipseButton = new QPushButton("Ellipse", this);
    m_ClearButton = new QPushButton("Clear", this);
    m_BezierCurveButton = new QPushButton("Bezier curve", this);
    m_FillColorButton = new QPushButton("Fill with color", this);
    m_ScanLineFillButton = new QPushButton("Filled polygon", this);


    m_lineButton->setGeometry(550,10,100,30);
    m_CircleButton->setGeometry(550,50,100,30);
    m_EllipseButton->setGeometry(550,90,100,30);
    m_ClearButton->setGeometry(550, 450, 100, 30);
    m_BezierCurveButton->setGeometry(550, 130, 100,30);
    m_FillColorButton->setGeometry(550, 170, 100, 30);
    m_ScanLineFillButton->setGeometry(550, 210, 100, 30);

    connect(m_lineButton, &QPushButton::clicked, this, &Ekran::setLineMode);
    connect(m_CircleButton, &QPushButton::clicked, this, &Ekran::setCircleMode);
    connect(m_EllipseButton, &QPushButton::clicked, this, &Ekran::setEllipseMode);
    connect(m_ClearButton, &QPushButton::clicked, this, &Ekran::clearAll);
    connect(m_BezierCurveButton, &QPushButton::clicked, this, &Ekran::setBezierCurveMode);
    connect(m_FillColorButton, &QPushButton::clicked, this, &Ekran::setFillWithColorMode);
    connect(m_ScanLineFillButton, &QPushButton::clicked, this, &Ekran::setScanLineFillMode);


    m_ellipseWindow = new EllipseWindow(nullptr);
    connect(m_ellipseWindow, &EllipseWindow::valueChanged, this, &Ekran::updateEllipseN);
}

Ekran::~Ekran()
{
    delete m_lineButton;
    delete m_CircleButton;
    delete m_EllipseButton;
    delete m_BezierCurveButton;
    delete m_FillColorButton;
    delete m_ClearButton;
    delete m_ellipseWindow;
    delete m_ScanLineFillButton;

    m_lineButton = nullptr;
    m_CircleButton = nullptr;
    m_EllipseButton = nullptr;
    m_BezierCurveButton = nullptr;
    m_FillColorButton = nullptr;
    m_ClearButton = nullptr;
    m_ellipseWindow = nullptr;
    m_ScanLineFillButton = nullptr;
}

void Ekran::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.fillRect(0, 0, width(), height(), Qt::yellow);
    p.drawImage(0, 0, m_canvas);

    //drawCircle(m_canvas, {100,100}, 3);
    //drawEllipse(m_canvas, {50,50}, {350,560}, 100);



    {
        for (const auto& circle: m_BezierCurvePoints)
            if (m_BezierCurvePoints.size() != 0)
        {
            drawCircle(m_canvas, circle.point, circle.radius);
        }

        if (m_BezierCurvePoints.size() >= 4 && (m_BezierCurvePoints.size() - 1) % 3 == 0)
        {
            int startIndex = 0;

            while (startIndex + 1 < m_BezierCurvePoints.size())
            {
                std::vector<BezierPoint> segmentPoints(
                    m_BezierCurvePoints.begin() + startIndex,
                    m_BezierCurvePoints.begin() + startIndex + 4
                    );
                drawBezierCurve(m_canvas, segmentPoints, 1000);

                startIndex += 3;
            }
        }
    }

    if (m_isDrawing)
    {
        Copy(m_canvas, m_tempImage);

        switch (m_mode)
        {
            case drawingMode::Line:
                    drawLineBresenham(m_tempImage, m_startPoint, m_endPoint);
                    break;
            case drawingMode::Circle:
                    drawCircle(m_tempImage, m_startPoint, m_endPoint);
                    break;
            case drawingMode::Ellipse:
                    drawEllipse(m_tempImage, m_startPoint, m_endPoint, m_ellipseN);
                    break;
            case drawingMode::BezierCurve:
                    //drawCircle(tempCanvas, m_startPoint, 3);
                    break;
            case drawingMode::FillWithColor:
                    break;
            case drawingMode::ScanLineFillMode:
                break;

        }

        p.drawImage(0, 0, m_tempImage);
    }

}

void Ekran::mouseMoveEvent(QMouseEvent *event)
{
    //QPoint p = event->pos();
    //int x = p.x();
    //int y = p.y();
    //drawPixel(canvas, x, y, 255, 255, 255);


    if (m_isDrawing)
    {
        m_endPoint = event->pos();
    }

    if (m_isDrawing && m_mode == drawingMode::BezierCurve)
    {
        if (m_isDraggedPoint && m_draggedPoint != -1)
        {
            QPoint newPos = event->pos();

            if (newPos.x() < 0) newPos.setX(0);
            else if (newPos.x() > m_canvas.width()) newPos.setX(m_canvas.width() - 1); // Maksymalna szerokość: width() - 1

            if (newPos.y() < 0) newPos.setY(0);
            else if (newPos.y() > m_canvas.height()) newPos.setY(m_canvas.height() - 1);


            m_BezierCurvePoints[m_draggedPoint].point = newPos;
            clear();
            //update();
        }
    }


    update();
}

void Ekran::mousePressEvent(QMouseEvent *event)
{

    if (event->button() == Qt::LeftButton)
    {
        m_startPoint = event->pos();
        m_endPoint = event->pos();
        m_isDrawing = true;


        if (m_mode == drawingMode::BezierCurve)
        {

            QPoint position = event->pos();

            auto comparator = [&position](const BezierPoint& a, const BezierPoint& b) {
                float distanceA = a.distanceSquared(position);
                float distanceB = b.distanceSquared(position);
                return distanceA < distanceB;
            };

            auto it = std::min_element(m_BezierCurvePoints.begin(), m_BezierCurvePoints.end(), comparator);

            if (it != m_BezierCurvePoints.end())
            {
                if (it->distanceSquared(position) <= it->radius * it->radius + 10)
                {
                    m_isDraggedPoint = true;
                    m_draggedPoint = std::distance(m_BezierCurvePoints.begin(), it);
                }
            }

        }

    }


    if (event->button() == Qt::MiddleButton && m_mode == drawingMode::BezierCurve)
    {

        QPoint position = event->pos();
        m_isDrawing = true;


        auto comparator = [&position](const BezierPoint& a, const BezierPoint& b) {
            float distanceA = a.distanceSquared(position);
            float distanceB = b.distanceSquared(position);
            return distanceA < distanceB;
        };

        auto it = std::min_element(m_BezierCurvePoints.begin(), m_BezierCurvePoints.end(), comparator);

        if (it != m_BezierCurvePoints.end())
        {
            if (it->distanceSquared(position) <= it->radius * it->radius + 10)
            {
                m_isDraggedPoint = true;
                m_draggedPoint = std::distance(m_BezierCurvePoints.begin(), it);
            }
        }
    }


    if ((event->button() == Qt::LeftButton) && (m_mode == drawingMode::ScanLineFillMode))
    {
        m_startPoint = event->pos();
        m_endPoint = event->pos();
        m_isDrawing = true;

        m_points.push_back(m_startPoint);



        if (m_points.size() > 1)
        {
            drawLineBresenham(m_canvas, m_points[m_points.size() - 2], m_points[m_points.size() - 1]);
        }
    }


    if ((event->button() == Qt::RightButton) && (m_mode == drawingMode::ScanLineFillMode))
    {
        drawLineBresenham(m_canvas, m_points[0], m_points[m_points.size() - 1]);
        scanLineFill(m_canvas, m_points);
    }

    if (event->button() == Qt::RightButton)
    {
        m_isDrawing = true;
        QPoint position = event->pos();

        removePoints(m_BezierCurvePoints, position);
    }

}

void Ekran::mouseReleaseEvent(QMouseEvent *event)
{

    PixelColor color{0, 0, 0, 0};

    if (m_isDraggedPoint)
    {
        m_isDraggedPoint = false;
        m_draggedPoint = -1;
        //clear();
        update();
    }


    if (event->button() == Qt::LeftButton && m_isDrawing)
    {
        m_endPoint = event->pos();
        switch (m_mode)
        {
            case drawingMode::Line:
                drawLineBresenham(m_canvas, m_startPoint, m_endPoint);
                break;
            case drawingMode::Circle:
                drawCircle(m_canvas, m_startPoint, m_endPoint);
                break;
            case drawingMode::Ellipse:
                drawEllipse(m_canvas, m_startPoint, m_endPoint, m_ellipseN);
                break;
            case drawingMode::BezierCurve:
                drawCircle(m_canvas, m_startPoint, 3);
                m_BezierCurvePoints.emplace_back(BezierPoint{m_startPoint, 3});


                if (m_BezierCurvePoints.size() >= 4 && (m_BezierCurvePoints.size() - 1) % 3 == 0)
                {
                    auto startIdx = m_BezierCurvePoints.size() - 4;
                    std::vector<BezierPoint> segmentPoints(
                        m_BezierCurvePoints.begin() + startIdx,
                        m_BezierCurvePoints.begin() + startIdx + 4
                        );
                    drawBezierCurve(m_canvas, segmentPoints, 1000);
                }

                break;
            case drawingMode::FillWithColor:

                color = getPixelColor(m_canvas, m_endPoint);

                flood_fill(m_canvas, m_endPoint, color, PixelColor{255, 0, 0, 0});
                break;

            case drawingMode::ScanLineFillMode:
                break;

        }

        m_isDrawing = false;
        update();
    }

}


void Ekran::drawPixel(QImage& img, int x, int y, int h_color)
{
    if (x < 0 || x >= img.width() || y < 0 || y >= img.height()) return;

    uchar* line = img.scanLine(y);
    line[4*x] = h_color & 0xFF; //blue
    line[4*x + 1] = (h_color >> 8) & 0xFF; //green
    line[4*x + 2] = (h_color >> 16) & 0xFF; //red
    line[4*x + 3] = 255; // alpha
}

void Ekran::drawPixel(QImage& img, const QPoint& point, const PixelColor& color)
{
    int x = point.x();
    int y = point.y();

    int red = color.R;
    int green = color.G;
    int blue = color.B;

    if (x < 0 || x >= img.width() || y < 0 || y >= img.height()) return;

    uchar* line = img.scanLine(y);
    line[4*x] = blue; //blue
    line[4*x + 1] = green; //green
    line[4*x + 2] = red; //red
    line[4*x + 3] = 255; // alpha
}


void Ekran::drawLine(QImage& img, const QPoint& first, const QPoint& second)
{

    int x1 = first.x();
    int x2 = second.x();
    int y1 = first.y();
    int y2 = second.y();

    if (std::abs(y2 - y1) == 0) //pozioma
    {
        if (x2 < x1)
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        int y = y1;

        for (int x = x1; x <= x2; x++)
        {
            drawPixel(img, x, y, 0xFF0000);
        }
    }
    else if (std::abs(y2 - y1) <= std::abs(x2 - x1)) // 0 < m <= 1
    {
        if (x2 < x1)
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        int dy = y2 - y1;
        int dx = x2 - x1;
        float m = dy / (float)dx;
        float y = y1;

        for (int x = x1; x <= x2; x++)
        {
            //y = m * (x - x1) + y1;
            drawPixel(img, x, std::round(y), 0xFF0000);
            y += m;
        }
    }
    else
    {

        if (y2 < y1)
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        int dy = y2 - y1;
        int dx = x2 - x1;
        float m = dx / (float)dy;
        float x = x1;

        if (dx == 0)
        {
            for (int y = y1; y <= y2; y++) // pionowa
            {
                //x = m * (y - y1) + y1;
                drawPixel(img, x, y, 0xFF0000);
            }
        }
        else
        {
            for (int y = y1; y <= y2; y++)
            {
                //x = m * (y - y1) + x1;
                drawPixel(img, std::round(x), y, 0xFF0000);
                x += m;
            }
        }
    }

    update();
}

void Ekran::drawLineBresenham(QImage& img, const QPoint& first, const QPoint& second)
{
    int x1 = first.x();
    int x2 = second.x();
    int y1 = first.y();
    int y2 = second.y();

    if (std::abs(y2 - y1) == 0) //pozioma
    {
        if (x2 < x1)
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        for (int x = x1; x <= x2; x++)
        {
            drawPixel(img, x, y1, 0x00FF00);
        }
    }
    else if (std::abs(x2 - x1) == 0) //pionowa
    {
        if (y2 < y1)
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        for (int y = y1; y <= y2; y++)
        {
            drawPixel(img, x1, y, 0x00FF00);
        }
    }
    else if (std::abs(y2 - y1) <= std::abs(x2 - x1))
    {
        if (x1 > x2)
        {
            std::swap(x1,x2);
            std::swap(y1, y2);
        }

        int dx = x2 - x1;
        int dy = y2 - y1;

        int m = (dy < 0) ? -1 : 1;
        dy *= m;
        int y = y1;
        int D = 2 * dy - dx;

        for (int x = x1; x <= x2; x++)
        {
            drawPixel(img, x, y, 0x00FF00);
            if (D > 0)
            {
                y += m;
                D -= 2 * dx;
            }
            D += 2 * dy;
        }
    }
    else
    {
        if (y1 > y2)
        {
            std::swap(x1,x2);
            std::swap(y1, y2);
        }

        int dx = x2 - x1;
        int dy = y2 - y1;

        int m = (dx < 0) ? -1 : 1;
        dx *= m;

        int x = x1;
        int D = 2 * dx - dy;

        for (int y = y1; y <= y2; y++)
        {
            drawPixel(img, x, y, 0x00FF00);
            if (D > 0)
            {
                x += m;
                D -= 2 * dy;
            }
            D += 2 * dx;
        }
    }

    update();
}

// -------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------------------------------

void Ekran::drawCircle(QImage& img, const QPoint& first, const QPoint& second)
{
    int x1 = first.x();
    int x2 = second.x();
    int y1 = first.y();
    int y2 = second.y();

    float r = std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));

    float L = std::round(r/std::sqrt(2));
    float y;
    for (int i = 0; i <= L; i++)
    {
        y = std::sqrt(r * r - i * i);

        // Pierwsza cwiartka
        drawPixel(img, i + x1, std::round(y) + y1, 0xFF0000);
        drawPixel(img, std::round(y) + x1, i + y1, 0xFF0000);

        // Druga cwiartka
        drawPixel(img, std::round(y) + x1, -i + y1 , 0xFF0000);
        drawPixel(img, i + x1, -std::round(y) + y1 , 0xFF0000);

        // Trzecia cwiartka
        drawPixel(img, -i + x1, -std::round(y) + y1 , 0xFF0000);
        drawPixel(img, -std::round(y) + x1, -i + y1 , 0xFF0000);

        // Czwarta cwiartka
        drawPixel(img, -i + x1, std::round(y) + y1 , 0xFF0000);
        drawPixel(img, -std::round(y) + x1, i + y1 , 0xFF0000);

    }

    update();
}

void Ekran::drawCircle(QImage &img, const QPoint &first, float r)
{
    int x1 = first.x();
    int y1 = first.y();

    float L = std::round(r/std::sqrt(2));
    float y;
    for (int i = 0; i <= L; i++)
    {
        y = std::sqrt(r * r - i * i);

        // Pierwsza cwiartka
        drawPixel(img, i + x1, std::round(y) + y1, 0xFF0000);
        drawPixel(img, std::round(y) + x1, i + y1, 0xFF0000);

        // Druga cwiartka
        drawPixel(img, std::round(y) + x1, -i + y1 , 0xFF0000);
        drawPixel(img, i + x1, -std::round(y) + y1 , 0xFF0000);

        // Trzecia cwiartka
        drawPixel(img, -i + x1, -std::round(y) + y1 , 0xFF0000);
        drawPixel(img, -std::round(y) + x1, -i + y1 , 0xFF0000);

        // Czwarta cwiartka
        drawPixel(img, -i + x1, std::round(y) + y1 , 0xFF0000);
        drawPixel(img, -std::round(y) + x1, i + y1 , 0xFF0000);

    }

    for (int i = -std::ceil(r); i <= std::ceil(r); i++)
    {
        for (int j = -std::ceil(r); j <= std::ceil(r); j++)
        {
            if (std::sqrt(j * j + i * i) <= r) {
                drawPixel(img, x1 + j, y1 + i, 0xFF00FF);
            }
        }
    }


    update();
}

void Ekran::drawEllipse(QImage &img, const QPoint &first, const QPoint &second, int N)
{
    int x1 = first.x();
    int x2 = second.x();
    int y1 = first.y();
    int y2 = second.y();

    int a = std::abs(y2 - y1) / 2; // pionowa
    int b = std::abs(x2 - x1) / 2; //pozioma

    float ox = (x1 + x2) /2.0;
    float oy = (y1 + y2) /2.0;

    for (int i = 0; i < N; i++)
    {
        float alpha1 = (2 * M_PI * i) / N;
        float alpha2 = (2 * M_PI * (i + 1)) / N;

        int x = ox + b * std::cos(alpha1);
        int y = oy + a * std::sin(alpha1);

        int xNext = ox + b * std::cos(alpha2);
        int yNext = oy + a * std::sin(alpha2);

        drawLineBresenham(img, {x, y}, {xNext, yNext});
    }

    update();
}


// -------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------------------------------


void Ekran::drawBezierCurve(QImage &img, const std::vector<BezierPoint> &controlPoints, int N)
{
    if (controlPoints.size() != 4)
    {
        qDebug() << "Potrzeba 4 punktow";
        return;
    }


    QPoint P0 = {controlPoints[0].point.x(), controlPoints[0].point.y()};
    QPoint P1 = {controlPoints[1].point.x(), controlPoints[1].point.y()};
    QPoint P2 = {controlPoints[2].point.x(), controlPoints[2].point.y()};
    QPoint P3 = {controlPoints[3].point.x(), controlPoints[3].point.y()};


    QPoint previousPoint = P0;

    for (int i = 0; i < N; i++)
    {
        float t = i / (N * 1.0);

        float x = std::pow(1 - t, 3)* P0.x() +
                  3 * std::pow(1 - t, 2)* t * P1.x() +
                  3 * (1 - t) * t * t * P2.x() +
                  t * t * t * P3.x();

        float y = std::pow(1 - t, 3)* P0.y() +
                  3 * std::pow(1 - t, 2)* t * P1.y() +
                  3 * (1 - t) * t * t * P2.y() +
                  t * t * t * P3.y();

        QPoint currentPoint = {int(x), int(y)};

        drawLineBresenham(m_canvas, previousPoint, currentPoint);

        previousPoint = currentPoint;
    }

    update();
}


// -------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------------------------------


void Ekran::flood_fill(QImage& img, const QPoint& point, const PixelColor& currentColor, const PixelColor& newColor)
{
    std::stack<QPoint> st;
    st.push(point);

    if (currentColor == newColor) return;

    while(!st.empty())
    {

        QPoint p = st.top();
        st.pop();


        if (p.x() < 0 || p.x() >= img.width() || p.y() < 0 || p.y() >= img.height()) continue;


        if (getPixelColor(img, p) == currentColor)
        {
            drawPixel(img, p, newColor);
            st.emplace(p.x() - 1, p.y());
            st.emplace(p.x() + 1, p.y());
            st.emplace(p.x(), p.y() - 1);
            st.emplace(p.x(), p.y() + 1);

        }
    }
}

// -------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------------------------------

void Ekran::scanLineFill(QImage &img, const std::vector<QPoint> &points)
{

    int yMax = std::max_element(points.begin(), points.end(), [](const QPoint& a, const QPoint& b) {
                                return  a.y() < b.y();
               })->y();

    int yMin = std::min_element(points.begin(), points.end(), [](const QPoint& a, const QPoint& b){
                                return a.y() < b.y();
               })->y();

    std::vector<int> intersections;

    for (int y = yMin; y <= yMax; y++)
    {
        intersections.clear();
        for (int i = 0; i < m_points.size(); i++)
        {
            QPoint A = m_points[i];
            QPoint B = m_points[(i + 1) % m_points.size()];

            //qDebug() << A;
            //qDebug() << B;


            // Spytaj sie o ten waruenk biore zawsze gorny a co sie dzieje z w przypadku V

            if ((A.y() <= y && B.y() > y ) || ((A.y() > y) && (B.y() <= y)))
            {
                int x = A.x() + (y - A.y()) * (B.x() - A.x()) / (B.y() - A.y());
                intersections.push_back(x);
            }


        }


        std::sort(intersections.begin(), intersections.end());

        for (int k = 0; k < intersections.size(); k+= 2)
        {

            if (k + 1 < intersections.size())
            {
                int xStart = intersections[k];
                int xEnd = intersections[k + 1];

                for (int x = xStart; x <= xEnd; x++)
                {
                    drawPixel(img, x, y, 0x00FF00);
                }
            }
        }
    }

}

// -------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------------------------------

PixelColor Ekran::getPixelColor(const QImage& img, const QPoint& point) const
{
    int x = point.x();
    int y = point.y();

    if (x < 0 || x >= img.width() || y < 0 || y >= img.height()) return {0, 0, 0, 0};

    const uchar* line = img.scanLine(y);
    uint8_t blue = line[4*x]; //blue
    uint8_t green = line[4*x + 1]; //green
    uint8_t red = line[4*x + 2]; //red
    uint8_t alpha = line[4*x + 3]; // alpha


    return PixelColor{red, green, blue, alpha};
}

// -------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------------------------------

void Ekran::clear()
{
    m_canvas.fill(0);
    update();
}

void Ekran::removePoints(std::vector<BezierPoint> &bezierPoints, const QPoint &position)
{
    auto comparator = [&position](const BezierPoint& a, const BezierPoint& b) {
        float distanceA = a.distanceSquared(position);
        float distanceB = b.distanceSquared(position);
        return distanceA < distanceB;
    };

    auto it = std::min_element(bezierPoints.begin(), bezierPoints.end(), comparator);

    if (it != bezierPoints.end())
    {
        if (it->distanceSquared(position) <= it->radius * it->radius + 10)
        {
            bezierPoints.erase(it);
            clear();
        }
    }
}

void Ekran::Copy(QImage &image, QImage &image2)
{

    if (image.size() != image2.size())
    {
        qWarning() << "Blad wymiarow";
        return;
    }

    for (int y = 0; y < image.height(); y++)
    {
        const uchar* line = image.scanLine(y);
        uchar* line2 = image2.scanLine(y);

        std::memcpy(line2, line, image.bytesPerLine());
    }
}

void Ekran::setLineMode()
{
    m_mode = drawingMode::Line;
    m_ellipseWindow->hide();
}

void Ekran::setCircleMode()
{
    m_mode = drawingMode::Circle;
    m_ellipseWindow->hide();
}

void Ekran::setEllipseMode()
{
    m_mode = drawingMode::Ellipse;

    QPoint EkranPos = this->pos();
    m_ellipseWindow->move(EkranPos.x() + 750, EkranPos.y() +50);

    m_ellipseWindow->show();
}

void Ekran::setBezierCurveMode()
{
    m_mode = drawingMode::BezierCurve;
    m_ellipseWindow->hide();
}

void Ekran::setFillWithColorMode()
{
    m_mode = drawingMode::FillWithColor;
    m_ellipseWindow->hide();
}

void Ekran::setScanLineFillMode()
{
    m_mode = drawingMode::ScanLineFillMode;
    m_ellipseWindow->hide();
}

void Ekran::clearAll()
{
    m_canvas.fill(0);
    update();
    m_isDrawing = false;

    m_BezierCurvePoints.clear();
    m_points.clear();

    m_ellipseWindow->hide();
}

void Ekran::updateEllipseN(int value)
{
    m_ellipseN = value;
}
