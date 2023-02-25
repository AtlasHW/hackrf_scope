#include "scopeview.h"

#include <QPainter>
#include <QPaintEvent>
#include <cmath>

//#define __SCOPEVIEW_DEBUG 1

#ifdef __SCOPEVIEW_DEBUG
#include <QDebug>
#endif

ScopeView::ScopeView(QWidget *parent) : QWidget(parent)
{
    time_line_show = false;
    trigger_line_show = false;
    CH_line_show = 0x00;

    time_drag_press_flag = false;
    trigger_drag_press_flag = false;
    CH1_drag_press_flag = false;
    CH2_drag_press_flag = false;
    CH3_drag_press_flag = false;
    CH4_drag_press_flag = false;
}

void ScopeView::paramentInit(scope_para_t *para)
{
    CH_EN = para->CH_EN;
    trigger_direct = para->trigger_direct;
    time_line_location = para->time_offset;
    trigger_line_location = para->trigger_location;
    CH1_offset = para->CH1_offset;
    CH2_offset = para->CH2_offset;
    CH3_offset = para->CH3_offset;
    CH4_offset = para->CH4_offset;
    TIME_solution = para->T_solution;
    trigger_label = para->trigger_label;
    CH1_solution = para->CH1_solution;
    CH2_solution = para->CH2_solution;
    CH3_solution = para->CH3_solution;
    CH4_solution = para->CH4_solution;
    update();
}

void ScopeView::paramentDefault()
{
    CH_EN = 0x03; // Channel 1 and channel 2 enable
    trigger_direct = 0; //rise
    time_line_location = 0;
    trigger_line_location = 0;
    CH1_offset = -200;
    CH2_offset = 200;
    CH3_offset = 0;
    CH4_offset = 0;

    TIME_solution = tr("10uS/DIV");
    trigger_label = tr("CH1 Normal");
    CH1_solution = tr("1V/DIV");
    CH2_solution = tr("1V/DIV");
    update();
}

void ScopeView::createDemoWave()
{
    CH_EN = 0x0f;
    trigger_direct = 2;
    time_line_location = 10;
    trigger_line_location = 120;
    CH1_offset = 300;
    CH2_offset = 100;
    CH3_offset = -100;
    CH4_offset = -300;

    CH1_solution=tr("1V/DIV");
    CH2_solution=tr("40V/DIV");
    CH3_solution=tr("1mV/DIV");
    CH4_solution=tr("900mV/DIV");
    TIME_solution=tr("10uS/DIV");
    trigger_label=tr("CH1 AUTO");

    bottom_label1 = tr("CH1 VPP:180mV");
    bottom_label2 = tr("CH1 FREQ:5kHz");
    bottom_label3 = tr("CH2 VMAX:11.1V");
    bottom_label4 = tr("CH1 DUTY:11%");

    for(int i=0; i<1000; i++)
    {

        CH1_draw_buffer << 100 * sin(i * 0.05 + M_PI);
        CH2_draw_buffer << 100 * sin(i * 0.02);
        CH3_draw_buffer << 100 * sin(i * 0.05);
        CH4_draw_buffer << 100 * sin(i * 0.05);
    }
    update();
}

void ScopeView::enableChannel(int channel)
{
    if(channel == 1)
        CH_EN = CH_EN | 0x01;
    if(channel == 2)
        CH_EN = CH_EN | 0x02;
    if(channel == 3)
        CH_EN = CH_EN | 0x04;
    if(channel == 4)
        CH_EN = CH_EN | 0x08;
    update();
}

void ScopeView::disableChannel(int channel)
{
    if(channel == 1)
        CH_EN = CH_EN & (~0x01);
    if(channel == 2)
        CH_EN = CH_EN & (~0x02);
    if(channel == 3)
        CH_EN = CH_EN & (~0x04);
    if(channel == 4)
        CH_EN = CH_EN & (~0x08);
    update();
}

void ScopeView::setCH_Enable(int channel, bool ena)
{
    if(ena)
        enableChannel(channel);
    else
        disableChannel(channel);
}

void ScopeView::setTimeSolution(QString solution)
{
    TIME_solution = solution;
    update();
}

void ScopeView::setTriggerDirection(uint8_t direction)
{
    trigger_direct = direction;
    update();
}

void ScopeView::setTriggerLabel(QString label)
{
    trigger_label = label;
    update();
}

void ScopeView::setCHLabel(int CH, QString label)
{
    if(CH == 1)
        CH1_solution = label;
    if(CH == 2)
        CH2_solution = label;
    if(CH == 3)
        CH2_solution = label;
    if(CH == 4)
        CH2_solution = label;
    update();
}

void ScopeView::setBottomLabel(int index, QString label)
{
    if(index == 1)
        bottom_label1 = label;
    if(index == 2)
        bottom_label2 = label;
    if(index == 3)
        bottom_label3 = label;
    if(index == 4)
        bottom_label4 = label;
    update();
}

void ScopeView::setBottomLabelBuffer(int index, QString label)
{
    if(index == 1)
        bottom_label1 = label;
    if(index == 2)
        bottom_label2 = label;
    if(index == 3)
        bottom_label3 = label;
    if(index == 4)
        bottom_label4 = label;
}

void ScopeView::paintEvent(QPaintEvent *event)
{
#ifdef __SCOPEVIEW_DEBUG
    qDebug() << "Paint event" <<event->rect().size();
    qDebug() << "Paint windwos size:" <<event->rect().size();
#endif

    QSize widget_size=event->rect().size();
    event->accept();

    int wave_area_Xoffset=20;
    int wave_area_Yoffset=20;
    wave_area_W=widget_size.width()-40;
    wave_area_H=widget_size.height()-60;

    QPainter painter(this);
    QPen pen;
    QVector<QPoint> point_set;

    //Channel Color
    QColor CH1_color("#6F6FFF");//sky blue
    QColor CH2_color("#00FF00");//green
    QColor CH3_color("#FF0000");//red
    QColor CH4_color("#F000F0");//purple
    QColor CH_DIS_color("#808080");//grave


    //paint backgraund
    painter.fillRect(rect(),QBrush(QColor("#111111")));

    //Paint outline
    pen.setColor(QColor("#F0F0F0"));
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawRect(20,20,widget_size.width()-40,widget_size.height()-60);

    //Paint center grid
    pen.setColor(QColor("#F0F0F0"));
    pen.setStyle(Qt::DotLine);
    pen.setWidth(2);
    painter.setPen(pen);

    painter.drawLine(wave_area_Xoffset, wave_area_Yoffset + wave_area_H / 2,
                     wave_area_Xoffset + wave_area_W, wave_area_Yoffset + wave_area_H / 2);

    painter.drawLine(wave_area_Xoffset + wave_area_W / 2, wave_area_Yoffset,
                     wave_area_Xoffset + wave_area_W / 2, wave_area_Yoffset + wave_area_H);

    //Paint Other grid
    pen.setColor(QColor("#F0F0F0"));
    pen.setStyle(Qt::DotLine);
    pen.setWidth(1);
    painter.setPen(pen);
    for(int i = 1; i <= 4; i++)
    {
        painter.drawLine(wave_area_Xoffset, wave_area_Yoffset + (wave_area_H * i) / 10,
                         wave_area_Xoffset + wave_area_W, wave_area_Yoffset + (wave_area_H * i) / 10);

        painter.drawLine(wave_area_Xoffset, wave_area_Yoffset + (wave_area_H * (i + 5 )) / 10,
                         wave_area_Xoffset + wave_area_W, wave_area_Yoffset + (wave_area_H * (i + 5)) / 10);
    }
    for(int i = 1; i <= 5; i++)
    {
        painter.drawLine(wave_area_Xoffset + (wave_area_W * i) / 12, wave_area_Yoffset,
                         wave_area_Xoffset + (wave_area_W * i) / 12, wave_area_Yoffset + wave_area_H);

        painter.drawLine(wave_area_Xoffset + (wave_area_W * (i + 6)) / 12, wave_area_Yoffset,
                         wave_area_Xoffset + (wave_area_W * (i + 6)) / 12, wave_area_Yoffset + wave_area_H);
    }

    //Paint top lable
    if(CH_EN & 0x01) //CH1 top lable
    {
        pen.setColor(CH1_color);
        painter.setPen(pen);
        painter.drawText(20, 15, tr("CH1 ") + CH1_solution);
    }
    else
    {
        pen.setColor(CH_DIS_color);
        painter.setPen(pen);
        painter.drawText(20, 15, tr("CH1"));
    }

    if(CH_EN & 0x02) //CH2 top lable
    {
        pen.setColor(CH2_color);
        painter.setPen(pen);
        painter.drawText(20 + 100, 15, tr("CH2 ") + CH2_solution);
    }
    else
    {
        pen.setColor(CH_DIS_color);
        painter.setPen(pen);
        painter.drawText(20 + 100, 15, tr("CH2"));
    }

    if(CH_EN & 0x04) //CH3 top lable
    {
        pen.setColor(CH3_color);
        painter.setPen(pen);
        painter.drawText(20 + 200, 15, tr("CH3 ") + CH3_solution);
    }
    else
    {
        pen.setColor(CH_DIS_color);
        painter.setPen(pen);
        painter.drawText(20 + 200, 15, tr("CH3"));
    }

    if(CH_EN & 0x08) //CH4 top lable
    {
        pen.setColor(CH4_color);
        painter.setPen(pen);
        painter.drawText(20 + 300, 15, tr("CH4 ") + CH4_solution);
    }
    else
    {
        pen.setColor(CH_DIS_color);
        painter.setPen(pen);
        painter.drawText(20 + 300, 15, tr("CH4"));
    }

    //paint time top lable
    pen.setColor(QColor("#FFFFFF"));
    painter.setPen(pen);
    painter.drawText(widget_size.width()-110, 15, tr("TIME ") + TIME_solution);

    //paint trigger bottom lable
    pen.setColor(QColor("#FFFFFF"));
    painter.setPen(pen);
    painter.drawText(widget_size.width()-170, wave_area_H + 35, tr("T:"));
    painter.drawText(widget_size.width()-135, wave_area_H + 35, trigger_label);

    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    QRect trigger_icon(widget_size.width()-155,widget_size.height()-38,16,16);
    painter.drawRect(trigger_icon);
    //fall icon
    if(trigger_direct != 0)
    {
        point_set.clear();
        point_set << trigger_icon.center() - QPoint(5, 5)
                  << trigger_icon.center() - QPoint(3, 5)
                  << trigger_icon.center() + QPoint(5, 7)
                  << trigger_icon.center() + QPoint(7, 7);
        painter.drawPolyline(point_set);
    }
    //rise icon
    if(trigger_direct != 1)
    {
        point_set.clear();
        point_set << trigger_icon.center() - QPoint(5, -7)
                  << trigger_icon.center() - QPoint(3, -7)
                  << trigger_icon.center() + QPoint(5, -5)
                  << trigger_icon.center() + QPoint(7, -5);
        painter.drawPolyline(point_set);
    }

    //paint other bottom lable
    pen.setColor(QColor("#FF7F00"));
    painter.setPen(pen);
    painter.drawText(20, wave_area_H + 35, bottom_label1);
    painter.drawText(220, wave_area_H + 35, bottom_label2);
    painter.drawText(20, wave_area_H + 55, bottom_label3);
    painter.drawText(220, wave_area_H + 55, bottom_label4);

    //paint time reference line
    if(time_line_location >= -500 && time_line_location <= 500) //in show range
    {
        int time_line_x = 20 + (time_line_location + 500) * wave_area_W / 1000;
        QRect ref_icon_rect(0, 0, 10, 8);
        ref_icon_rect.moveCenter(QPoint(time_line_x, 24));
        point_set.clear();
        point_set << ref_icon_rect.topLeft()
                  << ref_icon_rect.center() + QPoint(0, 4)
                  << ref_icon_rect.topRight()
                  << ref_icon_rect.topLeft();
        pen.setColor(QColor("#FFFFFF"));
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(QBrush(QColor("#FFFFFF")));
        painter.drawPolygon(point_set);

        if(time_line_show)
        {
            pen.setColor(QColor("#FFFFFF"));
            pen.setWidth(1);
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
            painter.drawLine(time_line_x, 20,
                             time_line_x, 20 + wave_area_H);//
        }
    }
    else if(time_line_location < -500) //out show range(left)
    {
        QRect ref_icon_rect(20, 20, 8, 10);
        point_set.clear();
        point_set << ref_icon_rect.topRight()
                  << ref_icon_rect.center() + QPoint(-4, 0)
                  << ref_icon_rect.bottomRight()
                  << ref_icon_rect.topRight();
        pen.setColor(QColor("#FFFFFF"));
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(QBrush(QColor("#FFFFFF")));
        painter.drawPolygon(point_set);
    }
    else if(time_line_location > 500) //out show range(right)
    {
        QRect ref_icon_rect(wave_area_W + 12, 20, 8, 10);
        point_set.clear();
        point_set << ref_icon_rect.topLeft()
                  << ref_icon_rect.center() + QPoint(4, 0)
                  << ref_icon_rect.bottomLeft()
                  << ref_icon_rect.topLeft();
        pen.setColor(QColor("#FFFFFF"));
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(QBrush(QColor("#FFFFFF")));
        painter.drawPolygon(point_set);
    }

    //paint trigger reference line
    if(trigger_line_location >= -500 && trigger_line_location <= 500) //in show range
    {
        int trigger_line_y = wave_area_H + 20 - (trigger_line_location + 500) * wave_area_H / 1000;
        QRect ref_icon_rect(0, 0, 8, 10);
        ref_icon_rect.moveCenter(QPoint(wave_area_W + 24, trigger_line_y));
        point_set.clear();
        point_set << ref_icon_rect.topRight()
                  << ref_icon_rect.center() + QPoint(-4, 0)
                  << ref_icon_rect.bottomRight()
                  << ref_icon_rect.topRight();
        pen.setColor(QColor("#FFFF00"));
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(QBrush(QColor("#FFFF00")));
        painter.drawPolygon(point_set);

        if(trigger_line_show)
        {
            pen.setColor(QColor("#FFFF00"));
            pen.setWidth(1);
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
            painter.drawLine(20, trigger_line_y,
                             20 + wave_area_W, trigger_line_y);//
        }
    }
    else if(trigger_line_location < -500) //out show range(down)
    {
        QRect ref_icon_rect(wave_area_W + 20, wave_area_H + 12, 10, 8);
        point_set.clear();
        point_set << ref_icon_rect.topRight()
                  << ref_icon_rect.center() + QPoint(0, 4)
                  << ref_icon_rect.topLeft()
                  << ref_icon_rect.topRight();
        pen.setColor(QColor("#FFFF00"));
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(QBrush(QColor("#FFFF00")));
        painter.drawPolygon(point_set);
    }
    else if(trigger_line_location > 500) //out show range(up)
    {
        QRect ref_icon_rect(wave_area_W + 20, 20, 10, 8);
        point_set.clear();
        point_set << ref_icon_rect.bottomLeft()
                  << ref_icon_rect.center() + QPoint(0, -4)
                  << ref_icon_rect.bottomRight()
                  << ref_icon_rect.bottomLeft();
        pen.setColor(QColor("#FFFF00"));
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(QBrush(QColor("#FFFF00")));
        painter.drawPolygon(point_set);
    }

    //Paint channel guide line
    if(CH_EN & 0x01) //Channel 1
    {
        if(CH1_offset >= -500 && CH1_offset <= 500) //in show range
        {
            int CH1_line_y = wave_area_H + 20 - (CH1_offset + 500) * wave_area_H / 1000;
            QRect ref_icon_rect(0, 0, 8, 10);
            ref_icon_rect.moveCenter(QPoint(16, CH1_line_y));
            point_set.clear();
            point_set << ref_icon_rect.topLeft()
                      << ref_icon_rect.center() + QPoint(4, 0)
                      << ref_icon_rect.bottomLeft()
                      << ref_icon_rect.topLeft();
            pen.setColor(CH1_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH1_color));
            painter.drawPolygon(point_set);

            if(CH_line_show & 0x01)
            {
                pen.setColor(CH1_color);
                pen.setWidth(1);
                pen.setStyle(Qt::DotLine);
                painter.setPen(pen);
                painter.drawLine(20, CH1_line_y,
                                 20 + wave_area_W, CH1_line_y);//
            }
        }
        else if(CH1_offset < -500) //out show range(down)
        {
            QRect ref_icon_rect(10, wave_area_H + 12, 10, 8);
            point_set.clear();
            point_set << ref_icon_rect.topRight()
                      << ref_icon_rect.center() + QPoint(0, 4)
                      << ref_icon_rect.topLeft()
                      << ref_icon_rect.topRight();
            pen.setColor(CH1_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH1_color));
            painter.drawPolygon(point_set);
        }
        else if(CH1_offset > 500) //out show range(up)
        {
            QRect ref_icon_rect(10, 20, 10, 8);
            point_set.clear();
            point_set << ref_icon_rect.bottomLeft()
                      << ref_icon_rect.center() + QPoint(0, -4)
                      << ref_icon_rect.bottomRight()
                      << ref_icon_rect.bottomLeft();
            pen.setColor(CH1_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH1_color));
            painter.drawPolygon(point_set);
        }
    }

    if(CH_EN & 0x02) //Channel 2
    {
        if(CH2_offset >= -500 && CH2_offset <= 500) //in show range
        {
            int CH2_line_y = wave_area_H + 20 - (CH2_offset + 500) * wave_area_H / 1000;
            QRect ref_icon_rect(0, 0, 8, 10);
            ref_icon_rect.moveCenter(QPoint(16, CH2_line_y));
            point_set.clear();
            point_set << ref_icon_rect.topLeft()
                      << ref_icon_rect.center() + QPoint(4, 0)
                      << ref_icon_rect.bottomLeft()
                      << ref_icon_rect.topLeft();
            pen.setColor(CH2_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH2_color));
            painter.drawPolygon(point_set);

            if(CH_line_show & 0x02)
            {
                pen.setColor(CH2_color);
                pen.setWidth(1);
                pen.setStyle(Qt::DotLine);
                painter.setPen(pen);
                painter.drawLine(20, CH2_line_y,
                                 20 + wave_area_W, CH2_line_y);//
            }
        }
        else if(CH2_offset < -500) //out show range(down)
        {
            QRect ref_icon_rect(10, wave_area_H + 12, 10, 8);
            point_set.clear();
            point_set << ref_icon_rect.topRight()
                      << ref_icon_rect.center() + QPoint(0, 4)
                      << ref_icon_rect.topLeft()
                      << ref_icon_rect.topRight();
            pen.setColor(CH2_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH2_color));
            painter.drawPolygon(point_set);
        }
        else if(CH2_offset > 500) //out show range(up)
        {
            QRect ref_icon_rect(10, 20, 10, 8);
            point_set.clear();
            point_set << ref_icon_rect.bottomLeft()
                      << ref_icon_rect.center() + QPoint(0, -4)
                      << ref_icon_rect.bottomRight()
                      << ref_icon_rect.bottomLeft();
            pen.setColor(CH2_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH2_color));
            painter.drawPolygon(point_set);
        }
    }

    if(CH_EN & 0x04) //Channel 3
    {
        if(CH3_offset >= -500 && CH3_offset <= 500) //in show range
        {
            int CH3_line_y = wave_area_H + 20 - (CH3_offset +500) * wave_area_H / 1000;
            QRect ref_icon_rect(0, 0, 8, 10);
            ref_icon_rect.moveCenter(QPoint(16, CH3_line_y));
            point_set.clear();
            point_set << ref_icon_rect.topLeft()
                      << ref_icon_rect.center() + QPoint(4, 0)
                      << ref_icon_rect.bottomLeft()
                      << ref_icon_rect.topLeft();
            pen.setColor(CH3_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH3_color));
            painter.drawPolygon(point_set);

            if(CH_line_show & 0x04)
            {
                pen.setColor(CH3_color);
                pen.setWidth(1);
                pen.setStyle(Qt::DotLine);
                painter.setPen(pen);
                painter.drawLine(20, CH3_line_y,
                                 20 + wave_area_W, CH3_line_y);//
            }
        }
        else if(CH3_offset < -500) //out show range(down)
        {
            QRect ref_icon_rect(10, wave_area_H + 12, 10, 8);
            point_set.clear();
            point_set << ref_icon_rect.topRight()
                      << ref_icon_rect.center() + QPoint(0, 4)
                      << ref_icon_rect.topLeft()
                      << ref_icon_rect.topRight();
            pen.setColor(CH3_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH3_color));
            painter.drawPolygon(point_set);
        }
        else if(CH3_offset > 500) //out show range(up)
        {
            QRect ref_icon_rect(10, 20, 10, 8);
            point_set.clear();
            point_set << ref_icon_rect.bottomLeft()
                      << ref_icon_rect.center() + QPoint(0, -4)
                      << ref_icon_rect.bottomRight()
                      << ref_icon_rect.bottomLeft();
            pen.setColor(CH3_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH3_color));
            painter.drawPolygon(point_set);
        }
    }

    if(CH_EN & 0x08) //Channel 4
    {
        if(CH4_offset >= -500 && CH4_offset <= 500) //in show range
        {
            int CH4_line_y = wave_area_H + 20 - (CH4_offset + 500) * wave_area_H / 1000;
            QRect ref_icon_rect(0, 0, 8, 10);
            ref_icon_rect.moveCenter(QPoint(16, CH4_line_y));
            point_set.clear();
            point_set << ref_icon_rect.topLeft()
                      << ref_icon_rect.center() + QPoint(4, 0)
                      << ref_icon_rect.bottomLeft()
                      << ref_icon_rect.topLeft();
            pen.setColor(CH4_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH4_color));
            painter.drawPolygon(point_set);

            if(CH_line_show & 0x08)
            {
                pen.setColor(CH4_color);
                pen.setWidth(1);
                pen.setStyle(Qt::DotLine);
                painter.setPen(pen);
                painter.drawLine(20, CH4_line_y,
                                 20 + wave_area_W, CH4_line_y);//
            }
        }
        else if(CH4_offset < -500) //out show range(down)
        {
            QRect ref_icon_rect(10, wave_area_H + 12, 10, 8);
            point_set.clear();
            point_set << ref_icon_rect.topRight()
                      << ref_icon_rect.center() + QPoint(0, 4)
                      << ref_icon_rect.topLeft()
                      << ref_icon_rect.topRight();
            pen.setColor(CH4_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH4_color));
            painter.drawPolygon(point_set);
        }
        else if(CH4_offset > 500) //out show range(up)
        {
            QRect ref_icon_rect(10, 20, 10, 8);
            point_set.clear();
            point_set << ref_icon_rect.bottomLeft()
                      << ref_icon_rect.center() + QPoint(0, -4)
                      << ref_icon_rect.bottomRight()
                      << ref_icon_rect.bottomLeft();
            pen.setColor(CH4_color);
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(CH4_color));
            painter.drawPolygon(point_set);
        }
    }

    //draw wave
    if((CH_EN & 0x01) && (CH1_draw_buffer.count() >= 1000))
    {
        point_set.clear();
        for(int i = 0;i < 1000; i++)
        {
            int x = 20 + wave_area_W * i / 1000;
            int y_show = CH1_draw_buffer.at(i) + CH1_offset + 500;
            if(y_show > 1000) y_show = 1000;
            if(y_show < 0) y_show = 0;
            int y = 20 + wave_area_H - y_show * wave_area_H / 1000;
            point_set << QPoint(x, y);
        }
        pen.setColor(CH1_color);
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawPolyline(point_set);
    }
    if((CH_EN & 0x02) && (CH2_draw_buffer.count() >= 1000))
    {
        point_set.clear();
        for(int i = 0;i < 1000; i++)
        {
            int x = 20 + wave_area_W * i / 1000;
            int y_show = CH2_draw_buffer.at(i) + CH2_offset + 500;
            if(y_show > 1000) y_show = 1000;
            if(y_show < 0) y_show = 0;
            int y = 20 + wave_area_H - y_show * wave_area_H / 1000;
            point_set << QPoint(x, y);
        }
        pen.setColor(CH2_color);
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawPolyline(point_set);
    }
    if((CH_EN & 0x04) && (CH3_draw_buffer.count() >= 1000))
    {
        point_set.clear();
        for(int i = 0;i < 1000; i++)
        {
            int x = 20 + wave_area_W * i / 1000;
            int y_show = CH3_draw_buffer.at(i) + CH3_offset + 500;
            if(y_show > 1000) y_show = 1000;
            if(y_show < 0) y_show = 0;
            int y = 20 + wave_area_H - y_show * wave_area_H / 1000;
            point_set << QPoint(x, y);
        }
        pen.setColor(CH3_color);
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawPolyline(point_set);
    }
    if((CH_EN & 0x08) && (CH4_draw_buffer.count() >= 1000))
    {
        point_set.clear();
        for(int i = 0;i < 1000; i++)
        {
            int x = 20 + wave_area_W * i / 1000;
            int y_show = CH4_draw_buffer.at(i) + CH4_offset + 500;
            if(y_show > 1000) y_show = 1000;
            if(y_show < 0) y_show = 0;
            int y = 20 + wave_area_H - y_show * wave_area_H / 1000;
            point_set << QPoint(x, y);
        }
        pen.setColor(CH4_color);
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawPolyline(point_set);
    }

    painter.end();

    //Reset mouse trigger area
    setTime_drag_rect(time_line_location);
    setTrigger_drag_rect(trigger_line_location);
    if(CH_EN & 0x01)
        setCH1_drag_rect(CH1_offset);
    if(CH_EN & 0x02)
        setCH2_drag_rect(CH2_offset);
    if(CH_EN & 0x04)
        setCH3_drag_rect(CH3_offset);
    if(CH_EN & 0x08)
        setCH4_drag_rect(CH4_offset);
}

void ScopeView::mousePressEvent(QMouseEvent *event)
{
//    mouse_press_flag = true;
    event->accept();
    if(Time_drag_rect.contains(event->pos()))
    {
#ifdef __SCOPEVIEW_DEBUG
        qDebug() << "Pressed time drag area!";
#endif
        time_drag_start_x =  event->pos().x();
        time_drag_press_flag = true;
    }
    else if(Trigger_drag_rect.contains(event->pos()))
    {
#ifdef __SCOPEVIEW_DEBUG
        qDebug() << "Pressed trigger drag area!";
#endif
        trigger_drag_start_y =  event->pos().y();
        trigger_drag_press_flag = true;
    }
    else if((CH_EN & 0x01) && CH1_drag_rect.contains(event->pos()))
    {
#ifdef __SCOPEVIEW_DEBUG
        qDebug() << "Pressed CH1 drag area!";
#endif
        CH1_drag_start_y =  event->pos().y();
        CH1_drag_press_flag = true;
    }
    else if((CH_EN & 0x02) && CH2_drag_rect.contains(event->pos()))
    {
#ifdef __SCOPEVIEW_DEBUG
        qDebug() << "Pressed CH2 drag area!";
#endif
        CH2_drag_start_y =  event->pos().y();
        CH3_drag_press_flag = true;
    }
    else if((CH_EN & 0x04) && CH3_drag_rect.contains(event->pos()))
    {
#ifdef __SCOPEVIEW_DEBUG
        qDebug() << "Pressed CH3 drag area!";
#endif
        CH3_drag_start_y =  event->pos().y();
        CH3_drag_press_flag = true;
    }
    else if((CH_EN & 0x08) && CH4_drag_rect.contains(event->pos()))
    {
#ifdef __SCOPEVIEW_DEBUG
        qDebug() << "Pressed CH4 drag area!";
#endif
        CH4_drag_start_y =  event->pos().y();
        CH4_drag_press_flag = true;
    }
}

void ScopeView::mouseMoveEvent(QMouseEvent *event)
{
    if(time_drag_press_flag)
    {
        int distance = (event->pos().x() - time_drag_start_x) * 1000 / wave_area_W;
#ifdef __SCOPEVIEW_DEBUG
        qDebug()<< "time drag distance:" << distance;
#endif
        emit timeDrag(distance);
    }
    if(trigger_drag_press_flag)
    {
        int distance = (event->pos().y() - trigger_drag_start_y) * (-1000) / wave_area_H;
#ifdef __SCOPEVIEW_DEBUG
        qDebug()<< "trigger drag distance:" << distance;
#endif
        emit triggerDrag(distance);
    }
    if(CH1_drag_press_flag)
    {
        int distance = (event->pos().y() - trigger_drag_start_y) * (-1000) / wave_area_H;
#ifdef __SCOPEVIEW_DEBUG
        qDebug()<< "CH1 drag distance:" << distance;
#endif
        emit CH1_Drag(distance);
    }
    if(CH2_drag_press_flag)
    {
        int distance = (event->pos().y() - trigger_drag_start_y) * (-1000) / wave_area_H;
#ifdef __SCOPEVIEW_DEBUG
        qDebug()<< "CH2 drag distance:" << distance;
#endif
        emit CH2_Drag(distance);
    }
    if(CH3_drag_press_flag)
    {
        int distance = (event->pos().y() - trigger_drag_start_y) * (-1000) / wave_area_H;
#ifdef __SCOPEVIEW_DEBUG
        qDebug()<< "CH3 drag distance:" << distance;
#endif
        emit CH3_Drag(distance);
    }
    if(CH4_drag_press_flag)
    {
        int distance = (event->pos().y() - trigger_drag_start_y) * (-1000) / wave_area_H;
#ifdef __SCOPEVIEW_DEBUG
        qDebug()<< "CH4 drag distance:" << distance;
#endif
        emit CH4_Drag(distance);
    }
}

void ScopeView::mouseReleaseEvent(QMouseEvent *event)
{
    if(time_drag_press_flag)
    {
        emit timeDragFinish();
        time_drag_press_flag = false;
    }
    if(trigger_drag_press_flag)
    {
        emit triggerDragFinish();
        trigger_drag_press_flag = false;
    }
    if(CH1_drag_press_flag)
    {
        emit CH1_DragFinish();
        CH1_drag_press_flag = false;
    }
    if(CH2_drag_press_flag)
    {
        emit CH2_DragFinish();
        CH2_drag_press_flag = false;
    }
    if(CH3_drag_press_flag)
    {
        emit CH3_DragFinish();
        CH3_drag_press_flag = false;
    }
    if(CH4_drag_press_flag)
    {
        emit CH4_DragFinish();
        CH4_drag_press_flag = false;
    }
    event->accept();
}

void ScopeView::setCH1_drag_rect(const int offset)
{
    int pix_location = 20 + (500 - offset) * wave_area_H / 1000;
    if(pix_location < 25) pix_location = 25;
    if(pix_location > wave_area_H + 15) pix_location = wave_area_H + 15;
    CH1_drag_rect = QRect(10, pix_location-5, 10, 10);
#ifdef __SCOPEVIEW_DEBUG
    qDebug() << "Set CH1 drage area:" << CH1_drag_rect;
#endif
}

void ScopeView::setCH2_drag_rect(const int offset)
{
    int pix_location = 20 + (500 - offset) * wave_area_H / 1000;
    if(pix_location < 25) pix_location = 25;
    if(pix_location > wave_area_H + 15) pix_location = wave_area_H + 15;
    CH2_drag_rect = QRect(10, pix_location-5, 10, 10);
#ifdef __SCOPEVIEW_DEBUG
    qDebug() << "Set CH2 drage area:" << CH2_drag_rect;
#endif
}

void ScopeView::setCH3_drag_rect(const int offset)
{
    int pix_location = 20 + (500 - offset) * wave_area_H / 1000;
    if(pix_location < 25) pix_location = 25;
    if(pix_location > wave_area_H + 15) pix_location = wave_area_H + 15;
    CH3_drag_rect = QRect(10, pix_location-5, 10, 10);
#ifdef __SCOPEVIEW_DEBUG
    qDebug() << "Set CH3 drage area:" << CH3_drag_rect;
#endif
}

void ScopeView::setCH4_drag_rect(const int offset)
{
    int pix_location = 20 + (500 - offset) * wave_area_H / 1000;
    if(pix_location < 25) pix_location = 25;
    if(pix_location > wave_area_H + 15) pix_location = wave_area_H + 15;
    CH4_drag_rect = QRect(10, pix_location-5, 10, 10);
#ifdef __SCOPEVIEW_DEBUG
    qDebug() << "Set CH4 drage area:" << CH4_drag_rect;
#endif
}

void ScopeView::setTrigger_drag_rect(const int location)
{
    int pix_location = 20 + (500 - location) * wave_area_H / 1000;
    if(pix_location < 25) pix_location = 25;
    if(pix_location > wave_area_H + 15) pix_location = wave_area_H + 15;
    Trigger_drag_rect = QRect(wave_area_W + 20, pix_location-5, 10, 10);
#ifdef __SCOPEVIEW_DEBUG
    qDebug() << "Set trigger drage area:" << Trigger_drag_rect;
#endif
}

void ScopeView::setTime_drag_rect(const int location)
{
    int pix_location = 20 + (location + 500) * wave_area_W / 1000;
    if(pix_location < 25) pix_location = 25;
    if(pix_location > wave_area_W + 15) pix_location = wave_area_W + 15;
    Time_drag_rect = QRect(pix_location - 5, 20, 10, 10);
#ifdef __SCOPEVIEW_DEBUG
    qDebug() << "Set Time drage area:" << Time_drag_rect;
#endif
}

void ScopeView::changeTimeLocation(int location)
{
    time_line_location = location;
    update();
}

void ScopeView::changeTriggerLocation(int location)
{
    trigger_line_location = location;
    update();
}

void ScopeView::changeCHOffset(int channel, int offset)
{
    if(channel == 1)
        CH1_offset = offset;
    if(channel == 2)
        CH2_offset = offset;
    if(channel == 3)
        CH3_offset = offset;
    if(channel == 4)
        CH4_offset = offset;
    update();
}

void ScopeView::changeData(uint8_t changeMask, QList<QVector<int> > data)
{
    int index = 0;
    if(changeMask & 0x01)
    {
        CH1_draw_buffer = data.at(index);
        index++;
    }
    if(changeMask & 0x02)
    {
        CH2_draw_buffer = data.at(index);
        index++;
    }
    if(changeMask & 0x04)
    {
        CH3_draw_buffer = data.at(index);
        index++;
    }
    if(changeMask & 0x08)
    {
        CH3_draw_buffer = data.at(index);
    }
    update();
}
