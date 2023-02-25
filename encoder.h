#ifndef ENCODER_H
#define ENCODER_H

#include <QObject>
#include <QDial>

class Encoder : public QDial
{
    Q_OBJECT
public:
    explicit Encoder(QWidget *parent = 0);

private:
    int pro_value;
    int outvalue;

    int times;

protected:
    void mousePressEvent(QMouseEvent *me) override;

signals:
    void encoderOutput(int);

private slots:
    void res_value_change(int value);

public slots:
    void resetValue();

};

#endif // ENCODER_H
