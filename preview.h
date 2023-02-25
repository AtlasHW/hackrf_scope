#ifndef PREVIEW_H
#define PREVIEW_H

#include <QWidget>


class PreView : public QWidget
{
    Q_OBJECT
public:
    explicit PreView(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

};

#endif // PREVIEW_H
