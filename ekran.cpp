#include "ekran.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

#include <cmath>

Ekran::Ekran(QWidget *parent)
    : QWidget{parent}, m_isDrawing(false), m_ellipseN(1000)
{
    m_canvas = QImage(500, 500, QImage::Format_RGB32);
    m_canvas.fill(0);

    setFixedSize(700,500);

    m_lineButton = new QPushButton("Line", this);
    m_CircleButton = new QPushButton("Circle", this);
    m_EllipseButton = new QPushButton("Ellipse", this);
    m_ClearButton = new QPushButton("Clear", this);
    m_BezierCurve = new QPushButton("Bezier curve", this);

    m_lineButton->setGeometry(550,10,100,30);
    m_CircleButton->setGeometry(550,50,100,30);
    m_EllipseButton->setGeometry(550,90,100,30);
    m_ClearButton->setGeometry(550, 450, 100, 30);
    m_BezierCurve->setGeometry(550, 130, 100,30);


    connect(m_lineButton, &QPushButton::clicked, this, &Ekran::setLineMode);
    connect(m_CircleButton, &QPushButton::clicked, this, &Ekran::setCircleMode);
    connect(m_EllipseButton, &QPushButton::clicked, this, &Ekran::setEllipseMode);
    connect(m_ClearButton, &QPushButton::clicked, this, &Ekran::clearAll);
    connect(m_BezierCurve, &QPushButton::clicked, this, &Ekran::setBezierCurveMode);

    //Spytaj sie co z destruktorem okok
    m_ellipseWindow = new EllipseWindow(nullptr);
    connect(m_ellipseWindow, &EllipseWindow::valueChanged, this, &Ekran::updateEllipseN);

}

void Ekran::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.fillRect(0, 0, width(), height(), Qt::yellow);
    p.drawImage(0, 0, m_canvas);

    //drawCircle(m_canvas, {100,100}, 3);
    //drawEllipse(m_canvas, {50,50}, {350,560}, 100);

    if (m_BezierCurvePoints.size() != 0)
    {
        for (const auto& circle: m_BezierCurvePoints)
        {
            drawCircle(m_canvas, circle.point, circle.radius);
        }
    }

    if (m_isDrawing)
    {
        QImage tempCanvas = m_canvas.copy();

        switch (m_mode)
        {
            case drawingMode::Line:
                    drawLineBresenham(tempCanvas, m_startPoint, m_endPoint);
                    break;
            case drawingMode::Circle:
                    drawCircle(tempCanvas, m_startPoint, m_endPoint);
                    break;
            case drawingMode::Ellipse:
                    drawEllipse(tempCanvas, m_startPoint, m_endPoint, m_ellipseN);
                    break;
            case drawingMode::BezierCurve:
                //drawCircle(tempCanvas, m_startPoint, 3);
                break;
        }

        //drawLine(tempCanvas, m_startPoint, m_endPoint);
        //drawLineBresenham(tempCanvas, m_startPoint, m_endPoint);
        //drawCircle(tempCanvas, m_startPoint, m_endPoint);

        p.drawImage(0, 0, tempCanvas);
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

    update();
}

void Ekran::mousePressEvent(QMouseEvent *event)
{

    if (event->button() == Qt::LeftButton)
    {
        m_startPoint = event->pos();
        m_endPoint = event->pos();
        m_isDrawing = true;
    }


    if (event->button() == Qt::RightButton)
    {
        m_isDrawing = true;
        QPoint position = event->pos();


        //Dodaj funkcje std::min
        for (auto it = m_BezierCurvePoints.begin(); it != m_BezierCurvePoints.end();)
        {
            float distance = (it->point.x() - position.x()) * (it->point.x() - position.x()) +
                             (it->point.y() - position.y()) * (it->point.y() - position.y());


            if (distance <= (it->radius * it->radius) + 10)
            {
                it = m_BezierCurvePoints.erase(it);
                clear();
                break;
            }
            else
            {
                it++;
            }
        }
    }

}

void Ekran::mouseReleaseEvent(QMouseEvent *event)
{
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
            drawPixel(img, x, y, 0x0000FF);
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
            drawPixel(img, x, std::round(y), 0x0000FF);
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
                drawPixel(img, x, y, 0x0000FF);
            }
        }
        else
        {
            for (int y = y1; y <= y2; y++)
            {
                //x = m * (y - y1) + x1;
                drawPixel(img, std::round(x), y, 0x0000FF);
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
            drawPixel(img, x, y1, 0xFF0000);
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
            drawPixel(img, x1, y, 0xFF0000);
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
            drawPixel(img, x, y, 0xFF0000);
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
            drawPixel(img, x, y, 0xFF0000);
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

void Ekran::clear()
{
    m_canvas.fill(0);
    update();
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

void Ekran::clearAll()
{
    m_canvas.fill(0);
    update();
    m_isDrawing = false;

    m_BezierCurvePoints.clear();

    m_ellipseWindow->hide();
}

void Ekran::updateEllipseN(int value)
{
    m_ellipseN = value;
}
