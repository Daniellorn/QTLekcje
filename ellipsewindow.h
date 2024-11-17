#ifndef ELLIPSEWINDOW_H
#define ELLIPSEWINDOW_H

#include <QWidget>
#include <QSlider>
#include <QVBoxLayout>
#include <QLabel>

class EllipseWindow : public QWidget
{
    Q_OBJECT
public:
    explicit EllipseWindow(QWidget *parent = nullptr);
    int getValue() const;



private:
    QSlider* m_slider;
    QLabel* m_label;


private slots:
    void updateLabel(int value);

signals:
    void valueChanged(int new_value);
};

#endif // ELLIPSEWINDOW_H
