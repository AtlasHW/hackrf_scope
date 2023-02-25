#include "oscilloscope.h"
#include "ui_oscilloscope.h"

#include <cmath>
#include <QDebug>
#include <QTime>

oscilloscope::oscilloscope(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::oscilloscope)
{
    //[1] Set ui
    ui->setupUi(this);
    setWindowTitle(tr("Oscilloscope"));

    //[2] scopeview initial
    scopeview = ui->scopeview;
    scopeview->paramentDefault();
//    scopeview->createDemoWave();

    //[3] connect signals and slots
    connect(ui->CH1_EN, &QCheckBox::toggled, this, &oscilloscope::CH1_setEnable);
    connect(ui->CH2_EN, &QCheckBox::toggled, this, &oscilloscope::CH2_setEnable);
    connect(ui->CH3_EN, &QCheckBox::toggled, this, &oscilloscope::CH3_setEnable);
    connect(ui->CH4_EN, &QCheckBox::toggled, this, &oscilloscope::CH4_setEnable);

    connect(ui->triggerDirect, SIGNAL(currentIndexChanged(int)), this, SLOT(triggerDirectSet(int)));
    connect(ui->triggerMode, SIGNAL(currentIndexChanged(int)), this, SLOT(triggerModeSet(int)));
    connect(ui->triggerCH, SIGNAL(currentIndexChanged(int)), this, SLOT(triggerCHSet(int)));

    connect(ui->timeScaleBTN, &QPushButton::clicked, this, &oscilloscope::timeScaleReset);
    connect(ui->timeOffsetBTN, &QPushButton::clicked, this, &oscilloscope::timeOffsetReset);
    connect(ui->triggerBTN, &QPushButton::clicked, this, &oscilloscope::triggerReset);
    connect(ui->CH1ScaleBTN, &QPushButton::clicked, this, &oscilloscope::CH1ScaleReset);
    connect(ui->CH2ScaleBTN, &QPushButton::clicked, this, &oscilloscope::CH2ScaleReset);
    connect(ui->CH3ScaleBTN, &QPushButton::clicked, this, &oscilloscope::CH3ScaleReset);
    connect(ui->CH4ScaleBTN, &QPushButton::clicked, this, &oscilloscope::CH4ScaleReset);
    connect(ui->CH1OffsetBTN, &QPushButton::clicked, this, &oscilloscope::CH1OffsetReset);
    connect(ui->CH2OffsetBTN, &QPushButton::clicked, this, &oscilloscope::CH2OffsetReset);
    connect(ui->CH3OffsetBTN, &QPushButton::clicked, this, &oscilloscope::CH3OffsetReset);
    connect(ui->CH4OffsetBTN, &QPushButton::clicked, this, &oscilloscope::CH4OffsetReset);

    connect(ui->timescaleENC, &Encoder::encoderOutput, this, &oscilloscope::timeScaleSet);
    connect(ui->timeoffsetENC, &Encoder::encoderOutput, this, &oscilloscope::timeOffsetSet);
    connect(ui->triggerENC, &Encoder::encoderOutput, this, &oscilloscope::triggerSet);
    connect(ui->CH1scaleENC, &Encoder::encoderOutput, this, &oscilloscope::CH1ScaleSet);
    connect(ui->CH2scaleENC, &Encoder::encoderOutput, this, &oscilloscope::CH2ScaleSet);
    connect(ui->CH3scaleENC, &Encoder::encoderOutput, this, &oscilloscope::CH3ScaleSet);
    connect(ui->CH4scaleENC, &Encoder::encoderOutput, this, &oscilloscope::CH4ScaleSet);
    connect(ui->CH1offsetENC, &Encoder::encoderOutput, this, &oscilloscope::CH1OffsetSet);
    connect(ui->CH2offsetENC, &Encoder::encoderOutput, this, &oscilloscope::CH2OffsetSet);
    connect(ui->CH3offsetENC, &Encoder::encoderOutput, this, &oscilloscope::CH3OffsetSet);
    connect(ui->CH4offsetENC, &Encoder::encoderOutput, this, &oscilloscope::CH4OffsetSet);


    view_update_timer=new QTimer(this);
    connect(view_update_timer,&QTimer::timeout,this,&oscilloscope::view_update);
    view_update_timer->start(100);

    rec_count=0;
    run_status=status_run;

    CH1_setting.channel_on=1;
    CH2_setting.channel_on=1;
    CH3_setting.channel_on=0;
    CH4_setting.channel_on=0;

    CH1_setting.channel_scale = 1;
    CH2_setting.channel_scale = 1;
    CH3_setting.channel_scale = 1;
    CH4_setting.channel_scale = 1;

    CH1_setting.channel_scale_code = 0;
    CH2_setting.channel_scale_code = 0;
    CH3_setting.channel_scale_code = 0;
    CH4_setting.channel_scale_code = 0;

    trigger_setting.trigger_mode = mode_normal;
    trigger_setting.trigger_level = 17;

    time_offset = 0;
}

oscilloscope::~oscilloscope()
{
    delete ui;
}

void oscilloscope::rx_data_update(scope_ch_data_t *ch_data)
{
    if((ch_data != NULL) && (ch_data->update_count != 0))
    {
        if(run_status == status_run)
        {
            int trigger_result=check_trigger(ch_data,&trigger_location);
            if(trigger_result==0 || trigger_setting.trigger_mode == mode_auto)
            {
                if(CH1_setting.channel_on && ch_data->ch1_data != NULL){
                    CH1_point_buffer.clear();
                    int index;
                    for(int i=0;i<1000;i++){
                        index = i * time_setting.time_scale;
                        CH1_point_buffer<<
                           *((ch_data->ch1_data) + ((trigger_location + index) % ch_data->buffer_size))
                                            * CH1_setting.channel_scale;
                    }
                }
                if(CH2_setting.channel_on && ch_data->ch2_data != NULL){
                    CH2_point_buffer.clear();
                    int index;
                    for(int i=0;i<1000;i++){
                        index = i * time_setting.time_scale;
                        CH2_point_buffer<<
                           *((ch_data->ch2_data) + ((trigger_location + index) % ch_data->buffer_size))
                                            * CH2_setting.channel_scale;
                    }
                }
                if(CH3_setting.channel_on && ch_data->ch3_data != NULL){
                    CH3_point_buffer.clear();
                    int index;
                    for(int i=0;i<1000;i++){
                        index = i * time_setting.time_scale;
                        CH3_point_buffer<<
                           *((ch_data->ch3_data) + ((trigger_location + index) % ch_data->buffer_size))
                                            * CH3_setting.channel_scale;
                    }
                }
                if(CH4_setting.channel_on && ch_data->ch4_data != NULL){
                    CH4_point_buffer.clear();
                    int index;
                    for(int i=0;i<1000;i++){
                        index = i * time_setting.time_scale;
                        CH4_point_buffer<<
                           *((ch_data->ch4_data) + ((trigger_location + index) % ch_data->buffer_size))
                                            * CH4_setting.channel_scale;
                    }
                }
            }
        }

        rec_count+=ch_data->update_count;
        ch_data->update_count=0;
    }
}

void oscilloscope::timeDragRes(int diff_value)
{
    int time_offset_temp = time_offset + diff_value;
    ui->scopeview->changeTimeLocation(time_offset_temp);
}

void oscilloscope::timeDragFinishRes()
{
//To do
}

void oscilloscope::view_update()
{
    static size_t pre_rec_count = 0;
    if(pre_rec_count != rec_count)
    {
        size_t update_count=rec_count-pre_rec_count;
        pre_rec_count=rec_count;
        qDebug()<<"Sample Rate(Khz):"<<(float)update_count/100000.0<<"Time: "<<QTime::currentTime().msecsSinceStartOfDay();
    }
    QList<QVector<int> > disply_data;
    uint8_t updata_mask = 0;

    if(CH1_setting.channel_on)
    {
        disply_data.append(CH1_point_buffer);
        updata_mask |= 0x01;
    }
    if(CH2_setting.channel_on)
    {
        disply_data.append(CH2_point_buffer);
        updata_mask |= 0x02;
    }
    if(CH3_setting.channel_on)
    {
        disply_data.append(CH3_point_buffer);
        updata_mask |= 0x04;
    }
    if(CH4_setting.channel_on)
    {
        disply_data.append(CH4_point_buffer);
        updata_mask |= 0x08;
    }

    scopeview->changeData(updata_mask, disply_data);
}

void oscilloscope::CH1_setEnable(bool value)
{
    CH1_setting.channel_on = value;
    scopeview->setCH_Enable(1, value);
}

void oscilloscope::CH2_setEnable(bool value)
{
    CH2_setting.channel_on = value;
    scopeview->setCH_Enable(2, value);
}

void oscilloscope::CH3_setEnable(bool value)
{
    CH3_setting.channel_on = value;
    scopeview->setCH_Enable(3, value);
}

void oscilloscope::CH4_setEnable(bool value)
{
    CH4_setting.channel_on = value;
    scopeview->setCH_Enable(4, value);
}

void oscilloscope::triggerDirectSet(int direct)
{
    if(direct == 0)
    {
        trigger_setting.trigger_direct = direrc_rise;
        scopeview->setTriggerDirection(0);
    }
    else if(direct == 1)
    {
        trigger_setting.trigger_direct = direrc_fall;
        scopeview->setTriggerDirection(1);
    }
    else
    {
        trigger_setting.trigger_direct = direrc_risefall;
        scopeview->setTriggerDirection(2);
    }
}

void oscilloscope::triggerModeSet(int mode)
{
    if(mode == 0)
        trigger_setting.trigger_mode = mode_normal;
    else
        trigger_setting.trigger_mode = mode_auto;
    update_trigger_label();
}

void oscilloscope::triggerCHSet(int CH)
{
    if(CH == 0)
    {
        trigger_setting.trigger_ch = CH1;
    }
    if(CH == 1)
    {
        trigger_setting.trigger_ch = CH2;
    }
    if(CH == 2)
    {
        trigger_setting.trigger_ch = CH3;
    }
    if(CH == 3)
    {
        trigger_setting.trigger_ch = CH4;
    }
    update_trigger_label();
}

void oscilloscope::timeScaleReset()
{
    time_setting.time_scale = 0;
    ui->timescaleENC->resetValue();
}

void oscilloscope::timeOffsetReset()
{
    time_setting.time_offset = 0;
    ui->timeoffsetENC->resetValue();
}

void oscilloscope::triggerReset()
{
    trigger_setting.trigger_level = 0;
    ui->triggerENC->resetValue();
}

void oscilloscope::CH1OffsetReset()
{
    CH1_setting.channel_offset = 0;
    ui->CH1offsetENC->resetValue();
}

void oscilloscope::CH2OffsetReset()
{
    CH2_setting.channel_offset = 0;
    ui->CH2offsetENC->resetValue();
}

void oscilloscope::CH3OffsetReset()
{
    CH3_setting.channel_offset = 0;
    ui->CH3offsetENC->resetValue();
}

void oscilloscope::CH4OffsetReset()
{
    CH4_setting.channel_offset = 0;
    ui->CH4offsetENC->resetValue();
}

void oscilloscope::CH1ScaleReset()
{
    CH1_setting.channel_scale_code = 0;
    CH1_setting.channel_scale = 1;
    ui->CH1scaleENC->resetValue();
}

void oscilloscope::CH2ScaleReset()
{
    CH2_setting.channel_scale_code = 0;
    CH2_setting.channel_scale = 1;
    ui->CH1scaleENC->resetValue();
}

void oscilloscope::CH3ScaleReset()
{
    CH3_setting.channel_scale_code = 0;
    CH3_setting.channel_scale = 1;
    ui->CH1scaleENC->resetValue();
}

void oscilloscope::CH4ScaleReset()
{
    CH4_setting.channel_scale_code = 0;
    CH4_setting.channel_scale = 1;
    ui->CH1scaleENC->resetValue();
}

void oscilloscope::timeScaleSet(int value)
{
    time_setting.time_scale_code += value;
    if(time_setting.time_scale_code < 0)
        time_setting.time_scale_code = 0;
    if(time_setting.time_scale_code > 100)
        time_setting.time_scale_code = 100;
    time_setting.time_scale = time_trans(time_setting.time_scale_code);
    //To deal time scale
}

void oscilloscope::timeOffsetSet(int value)
{
    time_setting.time_offset += value;
    //To test
    scopeview->changeTimeLocation(time_setting.time_offset);
}

void oscilloscope::triggerSet(int value)
{
    trigger_setting.trigger_level = value;
    scopeview->changeTriggerLocation(trigger_setting.trigger_level);
}

void oscilloscope::CH1OffsetSet(int value)
{
    CH1_setting.channel_offset += value;
    scopeview->changeCHOffset(1,CH1_setting.channel_offset);
}

void oscilloscope::CH2OffsetSet(int value)
{
    CH2_setting.channel_offset += value;
    scopeview->changeCHOffset(2,CH2_setting.channel_offset);
}

void oscilloscope::CH3OffsetSet(int value)
{
    CH2_setting.channel_offset += value;
    scopeview->changeCHOffset(3,CH3_setting.channel_offset);
}

void oscilloscope::CH4OffsetSet(int value)
{
    CH4_setting.channel_offset += value;
    scopeview->changeCHOffset(4,CH4_setting.channel_offset);
}

void oscilloscope::CH1ScaleSet(int value)
{
    CH1_setting.channel_scale_code += value;
    if(CH1_setting.channel_scale_code > 100)
        CH1_setting.channel_scale_code = 100;
    if(CH1_setting.channel_scale_code < -100)
        CH1_setting.channel_scale_code = -100;
    CH1_setting.channel_scale = level_trans(CH1_setting.channel_scale_code);
}

void oscilloscope::CH2ScaleSet(int value)
{
    CH2_setting.channel_scale_code += value;
    if(CH2_setting.channel_scale_code > 100)
        CH2_setting.channel_scale_code = 100;
    if(CH2_setting.channel_scale_code < -100)
        CH2_setting.channel_scale_code = -100;
    CH2_setting.channel_scale = level_trans(CH2_setting.channel_scale_code);
}

void oscilloscope::CH3ScaleSet(int value)
{
    CH3_setting.channel_scale_code += value;
    if(CH3_setting.channel_scale_code > 100)
        CH3_setting.channel_scale_code = 100;
    if(CH3_setting.channel_scale_code < -100)
        CH3_setting.channel_scale_code = -100;
    CH3_setting.channel_scale = level_trans(CH3_setting.channel_scale_code);
}

void oscilloscope::CH4ScaleSet(int value)
{
    CH4_setting.channel_scale_code += value;
    if(CH4_setting.channel_scale_code > 100)
        CH4_setting.channel_scale_code = 100;
    if(CH4_setting.channel_scale_code < -100)
        CH4_setting.channel_scale_code = -100;
    CH4_setting.channel_scale = level_trans(CH4_setting.channel_scale_code);
}

int oscilloscope::check_trigger(scope_ch_data_t *scope_ch_data, size_t *trigger_location)
{
    if(scope_ch_data->update_count == 0)
    {
        return -1;//No data uptate, no check
    }
    if((trigger_setting.trigger_level >= 128) || (trigger_setting.trigger_level <= -127))
    {
        return -2;//Trigger setting is illegal
    }
    int8_t *trigger_check_data;
    switch(trigger_setting.trigger_ch)
    {
    case CH1: trigger_check_data = (int8_t*)scope_ch_data->ch1_data; break;
    case CH2: trigger_check_data = (int8_t*)scope_ch_data->ch2_data; break;
    case CH3: trigger_check_data = (int8_t*)scope_ch_data->ch3_data; break;
    case CH4: trigger_check_data = (int8_t*)scope_ch_data->ch4_data; break;
    default:return -2;//Trigger setting is illegal
    }

    if(trigger_check_data == NULL)
    {
        return -3;//No data in this channel
    }

    int8_t pre_value = *(trigger_check_data + ((scope_ch_data->pre_index - 1) >0 ? (scope_ch_data->pre_index - 1) : (scope_ch_data->buffer_size - 1)));
    int trigger_status;//0 low or eq of trigger level, 1 high of trigger level
    if(pre_value <= trigger_setting.trigger_level)
        trigger_status = 0;
    else
        trigger_status = 1;

    if(trigger_setting.trigger_direct == direrc_risefall){
        size_t p = scope_ch_data->pre_index;
        while(p != scope_ch_data->update_index){
            if((trigger_status == 0 && *(trigger_check_data + p) > trigger_setting.trigger_level) ||
                    (trigger_status == 1 && *(trigger_check_data + p) <= trigger_setting.trigger_level))
            {
                *trigger_location = p;
                return 0;
            }
            p++;
            if(p >= scope_ch_data->buffer_size)
                p=0;
        }
    }
    else if(trigger_setting.trigger_direct == direrc_rise){
        size_t p = scope_ch_data->pre_index;
        while(p != scope_ch_data->update_index){
            if(trigger_status == 1)
            {
                if(*(trigger_check_data + p) <= trigger_setting.trigger_level)
                    trigger_status = 0;
            }
            else if(trigger_status == 0 && *(trigger_check_data + p) > trigger_setting.trigger_level)
            {
                *trigger_location = p;
                return 0;
            }
            p++;
            if(p >= scope_ch_data->buffer_size)
                p=0;
        }
    }
    else if(trigger_setting.trigger_direct == direrc_fall){
        size_t p = scope_ch_data->pre_index;
        while(p != scope_ch_data->update_index){
            if(trigger_status == 0)
            {
                if(*(trigger_check_data + p) > trigger_setting.trigger_level)
                    trigger_status = 1;
            }
            else if(trigger_status == 1 && *(trigger_check_data + p) <= trigger_setting.trigger_level)
            {
                *trigger_location = p;
                return 0;
            }
            p++;
            if(p >= scope_ch_data->buffer_size)
                p=0;
        }
    }
    return -100;//No trigger on this check
}

void oscilloscope::update_trigger_label()
{
    QString label;
    switch(static_cast<int>(trigger_setting.trigger_ch))
    {
    case CH1: label=tr("CH1");break;
    case CH2: label=tr("CH2");break;
    case CH3: label=tr("CH3");break;
    case CH4: label=tr("CH4");break;
    default: break;
    }
    if(trigger_setting.trigger_mode == mode_auto)
        label +=tr(" AUTO");
    else
        label += tr(" Normal");
    scopeview->setTriggerLabel(label);
}

float oscilloscope::time_trans(int value)
{
    float outvalue;
    outvalue = pow(10, value * 0.05f);
    return outvalue;
}

float oscilloscope::level_trans(int value)
{
    float outvalue;
    outvalue = pow(10, value * 0.05f);
    return outvalue;
}

