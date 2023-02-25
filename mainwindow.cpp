#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>
#include <QTime>
#include <cmath>

//Debug switch
#define _DEBUG_REC


#define BUFFER_SIZE 10000000
static int16_t rx_data_bufferI[BUFFER_SIZE];
static int16_t rx_data_bufferQ[BUFFER_SIZE];
static int16_t magnitude[BUFFER_SIZE];
size_t datalength=0;
scope_ch_data_t ch_data={
    .ch_count=3,
    .ch1_data=rx_data_bufferI,
    .ch2_data=rx_data_bufferQ,
    .ch3_data=magnitude,
    .ch4_data=NULL,
    .buffer_size=BUFFER_SIZE,
    .update_index=0,
    .pre_index=0,
    .update_count=0
};

static int rx_callback(hackrf_transfer* transfer) {
#ifdef _DEBUG_REC
    qDebug()<<QTime::currentTime().msecsSinceStartOfDay()<<transfer->valid_length;
#endif
    if(transfer->valid_length)
    {
        ch_data.pre_index = ch_data.update_index;
        for(int i = 0; i < transfer->valid_length; i += 2){
            *(rx_data_bufferI + ch_data.update_index) = (int) *((int8_t*)transfer->buffer + i);
            *(rx_data_bufferQ + ch_data.update_index) = (int) *((int8_t*)transfer->buffer + i + 1);
            *(magnitude + ch_data.update_index) = sqrt(pow(*(rx_data_bufferI + ch_data.update_index), 2)
                                                       + pow(*(rx_data_bufferQ + ch_data.update_index), 2));
            ch_data.update_index++;
            if(ch_data.update_index == ch_data.buffer_size)
                ch_data.update_index = 0;
        }
        datalength+=transfer->valid_length;
        ch_data.update_count += (transfer->valid_length / 2);
    }
    return 0;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //main_t();

    int result = HACKRF_SUCCESS;
    result = hackrf_init_t();
    if (result != HACKRF_SUCCESS) {
        QMessageBox::warning(this,tr("ERROR"),tr("Error:%1 Code:%2").arg(hackrf_error_name((hackrf_error)result)).arg(result));
    }

    for(int i = 0; i <= 40; i += 8)
    {
        ui->LNA_CB->addItem(tr("%1").arg(i));
    }

    for(int i = 0; i <= 62; i += 2)
    {
        ui->VGA_CB->addItem(tr("%1").arg(i));
    }

    datarx_timer=new QTimer(this);
    connect(datarx_timer,&QTimer::timeout,this,&MainWindow::datarx_slot);
    datarx_timer->start(33);
}

MainWindow::~MainWindow()
{
    hackrf_close_t();
    delete ui;
}

void MainWindow::datarx_slot()
{
    if(datalength){
        emit rx_data_update(&ch_data);
        qDebug()<<QTime::currentTime().msecsSinceStartOfDay()<<datalength;
        datalength=0;
    }
}

void MainWindow::on_pushButton_clicked()
{
    scope=new oscilloscope(this);
    scope->show();
    connect(this,&MainWindow::rx_data_update,scope,&oscilloscope::rx_data_update);
}

int MainWindow::hackrf_init_t()
{
    int result = hackrf_init();
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_init() failed: %s (%d)\n",
                hackrf_error_name((hackrf_error)result), result);
        return result;
    }
    result=hackrf_open(&device);
    if(result != HACKRF_SUCCESS){
        fprintf(stderr, "hackrf_open() failed: %s (%d)\n",
                hackrf_error_name((hackrf_error)result), result);
        return result;
    }

    result = hackrf_set_sample_rate(device, 10000000);
    if( result != HACKRF_SUCCESS ) {
        fprintf(stderr, "hackrf_set_sample_rate() failed: %s (%d)\n",
                hackrf_error_name((hackrf_error)result), result);
        return result;
    }

    result = hackrf_set_baseband_filter_bandwidth(device, 10000000);
    if( result != HACKRF_SUCCESS ) {
        fprintf(stderr, "hackrf_baseband_filter_bandwidth_set() failed: %s (%d)\n",
                hackrf_error_name((hackrf_error)result), result);
        return result;
    }

    result = hackrf_set_hw_sync_mode(device, 0);
    if( result != HACKRF_SUCCESS ) {
        fprintf(stderr, "hackrf_set_hw_sync_mode() failed: %s (%d)\n",
                hackrf_error_name((hackrf_error)result), result);
        return result;
    }

    result = hackrf_set_vga_gain(device, 20);
    result |= hackrf_set_lna_gain(device, 8);
    result |= hackrf_start_rx(device, rx_callback, NULL);
    if( result != HACKRF_SUCCESS ) {
        fprintf(stderr, "hackrf_start_rx() failed: %s (%d)\n",
                hackrf_error_name((hackrf_error)result), result);
        return result;
    }

    result = hackrf_set_freq(device, 315000000);
    if( result != HACKRF_SUCCESS ) {
        fprintf(stderr, "hackrf_set_freq() failed: %s (%d)\n",
                hackrf_error_name((hackrf_error)result), result);

        return result;
    }

    return HACKRF_SUCCESS;
}

int MainWindow::hackrf_close_t()
{
    if(device == NULL)
    {
        qDebug()<<"No device, exit!";
        hackrf_exit();
        return HACKRF_ERROR_NOT_FOUND;
    }
    int result = hackrf_stop_rx(device);
    if( result != HACKRF_SUCCESS ) {
        fprintf(stderr, "hackrf_stop_rx() failed: %s (%d)\n",
                hackrf_error_name((hackrf_error)result), result);
    } else {
        fprintf(stderr, "hackrf_stop_rx() done\n");
    }

     result = hackrf_close(device);
    if(result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_close() failed: %s (%d)\n", hackrf_error_name((hackrf_error)result), result);
    } else {
        fprintf(stderr, "hackrf_close() done\n");
    }
    hackrf_exit();
            fprintf(stderr, "hackrf_exit() done\n");
    return HACKRF_SUCCESS;
}

void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{
    if(device != NULL)
        hackrf_set_freq(device, static_cast<long long>(arg1*1000000));
}

void MainWindow::on_LNA_CB_currentTextChanged(const QString &arg1)
{
    int lna = arg1.toInt();
    if(device != NULL)
        hackrf_set_lna_gain(device, lna);
}

void MainWindow::on_VGA_CB_currentTextChanged(const QString &arg1)
{
    int vga = arg1.toInt();
    if(device != NULL)
        hackrf_set_vga_gain(device, vga);
}
