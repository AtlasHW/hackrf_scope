#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>
#include <QTime>
#include <cmath>

#include <pthread.h>

//Debug switch
//#define _DEBUG_REC


#define BUFFER_SIZE 10000000
static int16_t rx_data_bufferI[BUFFER_SIZE];
static int16_t rx_data_bufferQ[BUFFER_SIZE];
static int16_t magnitude[BUFFER_SIZE];

scope_ch_data_t ch_data = {
    .ch_mask = CH1_MASK | CH2_MASK | CH3_MASK,
    .ch1_data = rx_data_bufferI,
    .ch2_data = rx_data_bufferQ,
    .ch3_data = magnitude,
    .ch4_data = NULL,
    .buffer_size = BUFFER_SIZE,
    .head = 0,
    .tail = 0,
    .valid_length = 0,
    .overflow_flag = false
};

pthread_mutex_t mutex_lock;

static int rx_callback(hackrf_transfer* transfer) {
    if(ch_data.overflow_flag)
        return -1;

    if(transfer->valid_length)
    {
        pthread_mutex_lock(&mutex_lock);
        int16_t I,Q;
        for(int i = 0; i < transfer->valid_length; i += 2){
            I = *((int8_t*)transfer->buffer + i);
            Q = *((int8_t*)transfer->buffer + i + 1);
            *(rx_data_bufferI + ch_data.tail) = I;
            *(rx_data_bufferQ + ch_data.tail) = Q;
            *(magnitude + ch_data.tail) = sqrt(pow(I, 2) + pow(Q, 2));
            ch_data.tail++;
            if(ch_data.tail == ch_data.buffer_size)
                ch_data.tail = 0;
            if(ch_data.tail == ch_data.head) //Receive overflow
            {
                ch_data.overflow_flag = true;
                qDebug() << "Receive data flow!";
            }
        }
        ch_data.valid_length = ch_data.tail > ch_data.head ?
                               ch_data.tail - ch_data.head:
                               ch_data.tail + ch_data.buffer_size - ch_data.head;
        pthread_mutex_unlock(&mutex_lock);
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
        device_run = false;
        QMessageBox::warning(this,tr("ERROR"),tr("Error:%1 Code:%2").arg(hackrf_error_name((hackrf_error)result)).arg(result));
    }
    else
    {
        device_run = true;
    }

    for(int i = 0; i <= 40; i += 8)
    {
        ui->LNA_CB->addItem(tr("%1").arg(i));
    }
    ui->LNA_CB->setCurrentIndex(1);

    for(int i = 0; i <= 62; i += 2)
    {
        ui->VGA_CB->addItem(tr("%1").arg(i));
    }
    ui->VGA_CB->setCurrentIndex(10);

    datarx_timer=new QTimer(this);
    connect(datarx_timer,&QTimer::timeout,this,&MainWindow::datarx_slot);
    datarx_timer->start(33);

    pthread_mutex_init(&mutex_lock, NULL);
}

MainWindow::~MainWindow()
{
    hackrf_close_t();
    delete ui;
    pthread_mutex_destroy(&mutex_lock);
}

void MainWindow::datarx_slot()
{
    if(ch_data.valid_length){
        pthread_mutex_lock(&mutex_lock);
        emit rx_data_update(&ch_data);
        clear_rxch_buff(&ch_data);
        pthread_mutex_unlock(&mutex_lock);
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
    if(device_run == false)
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

int MainWindow::clear_rxch_buff(scope_ch_data_t *ch_buffer)
{
    if(ch_buffer->overflow_flag)
    {
        ch_buffer->overflow_flag = false;
        ch_buffer->head = 0;
        ch_buffer->tail = 0;
        ch_buffer->valid_length = 0;
        return 1;
    }
    else if(ch_buffer->valid_length > 0)
    {
        ch_buffer->head = ch_buffer->tail;
        ch_buffer->valid_length = 0;
        return 0;
    }
    return -1;
}

void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{
    if(device_run)
        hackrf_set_freq(device, static_cast<long long>(arg1*1000000));
}

void MainWindow::on_LNA_CB_currentTextChanged(const QString &arg1)
{
    int lna = arg1.toInt();
    if(device_run)
        hackrf_set_lna_gain(device, lna);
}

void MainWindow::on_VGA_CB_currentTextChanged(const QString &arg1)
{
    int vga = arg1.toInt();
    if(device_run)
        hackrf_set_vga_gain(device, vga);
}
