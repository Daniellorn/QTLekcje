#include "ellipsewindow.h"

EllipseWindow::EllipseWindow(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    setFixedSize(250,50);

    m_label = new QLabel("N: 1000", this);
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 1000);
    m_slider->setValue(500);

    layout->addWidget(m_label);
    layout->addWidget(m_slider);

    connect(m_slider, &QSlider::valueChanged, this, &EllipseWindow::updateLabel);
    connect(m_slider, &QSlider::valueChanged, this, &EllipseWindow::valueChanged);

    setLayout(layout);
    setWindowTitle("Podaj N");
}

int EllipseWindow::getValue() const
{
    return m_slider->value();
}

void EllipseWindow::updateLabel(int value)
{
    m_label->setText(QString("N: %1").arg(value));
}
