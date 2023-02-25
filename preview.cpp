#include "preview.h"

#include <QPainter>

PreView::PreView(QWidget *parent) : QWidget(parent)
{

}

void PreView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen pen;
    painter.fillRect(rect(),QBrush(QColor("#111111")));
}
