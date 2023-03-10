#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <hackrf.h>
#include <QTimer>
#include "oscilloscope.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void datarx_slot();

    void on_pushButton_clicked();

    void on_doubleSpinBox_valueChanged(double arg1);

    void on_LNA_CB_currentTextChanged(const QString &arg1);

    void on_VGA_CB_currentTextChanged(const QString &arg1);

private:
    int hackrf_init_t();
    int hackrf_close_t();

    int clear_rxch_buff(scope_ch_data_t *ch_data);

private:
    Ui::MainWindow *ui;

    hackrf_device* device;

    oscilloscope *scope;

    QTimer *datarx_timer;

    bool device_run;

signals:
    void rx_data_update(scope_ch_data_t*);
};

#endif // MAINWINDOW_H
