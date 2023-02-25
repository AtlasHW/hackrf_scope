#include "encoder.h"

#include <QMouseEvent>

//#define __ENC_DEBUG

#ifdef __ENC_DEBUG
#include <QDebug>
#endif


Encoder::Encoder(QWidget *parent) :
    QDial(parent)
{
    outvalue = 0;
    pro_value = 0;
    connect(this, &QDial::valueChanged, this, &Encoder::res_value_change);

    times = 10;
}

void Encoder::mousePressEvent(QMouseEvent *me)
{
    if(me->button() == Qt::RightButton)
    {
        if(times == 10)
        {
            times = 1;
            this->setPalette(QPalette(QColor("#D0D0D0")));
        }
        else
        {
            times = 10;
            this->setPalette(QPalette(QColor("#EFEFEF")));
        }
    }
    me->accept();
}

void Encoder::res_value_change(int value)
{
    if(pro_value != value)
    {
        outvalue = value - pro_value;
        if(outvalue > 50)
            outvalue -= 100;
        else if(outvalue < -50)
            outvalue += 100;

        pro_value = value;
        emit encoderOutput(outvalue * times);
#ifdef __ENC_DEBUG
        qDebug()<<this->objectName()<<" output value:"<<outvalue;
#endif
    }
}

void Encoder::resetValue()
{
    setValue(0);
    pro_value = 0;
    outvalue = 0;
    emit encoderOutput(outvalue);
}
