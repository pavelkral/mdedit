#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>
#include <QColor>
#include <QDialog>

class QSlider;
class QLabel;
class QPushButton;
class QLineEdit;


class ColorPicker : public QDialog
{
    Q_OBJECT

public:
    explicit ColorPicker(QWidget *parent = nullptr);

    ~ColorPicker();

    QColor color() const;

signals:
    void colorChanged(const QColor &color);

public slots:
    void setColor(const QColor &color);

private slots:
    void updateColorFromSlidersR();
    void updateColorFromSlidersB();
    void updateColorFromSlidersG();
    void pickColorFromScreen(); //

private:
    void updateColorDisplay();

    QSlider *m_redSlider;
    QSlider *m_greenSlider;
    QSlider *m_blueSlider;
    QLabel *m_colorDisplayLabel;
    QLabel *m_redValueLabel;
    QLabel *m_greenValueLabel;
    QLabel *m_blueValueLabel;
    QPushButton *m_pickColorButton;
    QLineEdit *m_htmlColorEdit;
    QLineEdit *m_rgbColorEdit;
    QLineEdit *m_hslColorEdit;
    QColor m_currentColor;


};

#endif // COLORPICKER_H
