#include "oscilloscope.h"
#include "ui_oscilloscope.h"

#include <cmath>
#include <QDebug>
#include <QTime>



oscilloscope::oscilloscope(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::oscilloscope)
{
    //[0] Initialize buffer
    //[0.1] Initialize L2 cache
    L2_cache = new scope_L2_cache_t;
    L2_cache->head = 0;
    L2_cache->tail = 0;
    L2_cache->frame_head = 0;
    L2_cache->frame_tail = 0;
    L2_cache->fetch_finish = true;
    L2_cache->show_valid = true;

    //[0.2] set time default parament
    time_setting.time_scale_code = 0;
    time_setting.time_scale = 1;
    time_setting.time_offset = 0;
    update_time_parament();

    //[0.3] set trigger
    trigger_setting.trigger_ch = CH1;
    trigger_setting.trigger_mode = mode_normal;
    trigger_setting.trigger_level = 0;
    trigger_setting.trigger_direct = direrc_rise;

    //[0.4] channel setting
    CH1_setting.channel_on=1;
    CH2_setting.channel_on=1;
    CH3_setting.channel_on=0;
    CH4_setting.channel_on=0;

    CH1_setting.channel_offset = 200;
    CH2_setting.channel_offset = -200;
    CH3_setting.channel_offset = 0;
    CH4_setting.channel_offset = 0;

    CH1_setting.channel_scale = 1;
    CH2_setting.channel_scale = 1;
    CH3_setting.channel_scale = 1;
    CH4_setting.channel_scale = 1;

    CH1_setting.channel_scale_code = 0;
    CH2_setting.channel_scale_code = 0;
    CH3_setting.channel_scale_code = 0;
    CH4_setting.channel_scale_code = 0;

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

    //[4] scope view update timer setting
    view_update_timer=new QTimer(this);
    connect(view_update_timer,&QTimer::timeout,this,&oscilloscope::view_update);
    view_update_timer->start(100);

    //[5] Other, set run status, set view parament

    run_status=status_run;

    scopeview->changeCHOffset(1, CH1_setting.channel_offset);
    scopeview->changeCHOffset(2, CH2_setting.channel_offset);
    update_trigger_location();
}

oscilloscope::~oscilloscope()
{
    delete ui;
}

/*
 * -: data of before trigger    =:data of after trigger
 *
 * |-----------------[-------------@===========]--------|  <-databuffer(1)
 *                 head        trigger       tail
 *
 * |=============]--------------[-------------@=========|  <-databuffer(2)
 *             tail           head         trigger
 */

void oscilloscope::rx_data_update(scope_ch_data_t *ch_data)
{
    if((ch_data == NULL) || (ch_data->valid_length == 0))
        return;

    update_trigger_parament(ch_data);

    if(run_status == status_run || (!L2_cache->show_valid))
    {
        L2_cache->save_count = 0;
        for(size_t i = 0; i < ch_data->valid_length; i++)
        {
            size_t data_index = (ch_data->head + i) % ch_data->buffer_size;

            //Check trigger when:
            //1. ckeck trigger flag is setted
            //2. no pending receive show frame data
            //3. show programme finished fetch data
            if(trigger.check_trigger_flag && L2_cache->show_valid && L2_cache->fetch_finish)
            {
                if(check_trigger(data_index))
                {
                    if(single_trigger_flag)
                        run_status = status_stop;

                    //current trigger loaction
                    L2_cache->trigger_location = L2_cache->tail;

                    //don't check trigger util the laction
                    trigger.check_trigger_flag = false;
                    L2_cache->next_trigger_check_location = (L2_cache->trigger_location + trigger.trigger_gap)
                                                            % L2_cache->buffer_size;
                    //calculate the display frame head and tail
                    long temp_frame_head = L2_cache->trigger_location - (time_setting.time_offset + 500) * oversample_rate;
                    if(temp_frame_head < 0)
                        temp_frame_head += L2_cache->buffer_size;
                    L2_cache->frame_head = temp_frame_head % L2_cache->buffer_size;
                    L2_cache->frame_tail = (L2_cache->frame_head + static_cast<size_t>(1000 * oversample_rate)) % L2_cache->buffer_size;

                    if(time_setting.time_offset > 500) //The current data has be enough, no waiting
                    {
                        L2_cache->show_valid = true;
                        L2_cache->fetch_finish = false;
                    }
                    else
                        L2_cache->show_valid = false;

                    qDebug()<<"Triggered! trigger loaction:" << L2_cache->trigger_location;
                    qDebug()<<"Frame head,tail:" << L2_cache->frame_head << L2_cache->frame_tail;
                    qDebug()<<"oversample:" << oversample_rate;
                    qDebug()<<"show valid:" << L2_cache->show_valid;
                }
            }

            if(i / datagap >= L2_cache->save_count + 1)
            {
                L2_cache->save_count++;

                if(CH1_setting.channel_on && ch_data->ch1_data != NULL)
                    L2_cache->CH_buffer[0][L2_cache->tail] = *(ch_data->ch1_data + data_index);
                if(CH2_setting.channel_on && ch_data->ch2_data != NULL)
                    L2_cache->CH_buffer[1][L2_cache->tail] = *(ch_data->ch2_data + data_index);
                if(CH3_setting.channel_on && ch_data->ch3_data != NULL)
                    L2_cache->CH_buffer[2][L2_cache->tail] = *(ch_data->ch3_data + data_index);
                if(CH4_setting.channel_on && ch_data->ch4_data != NULL)
                    L2_cache->CH_buffer[3][L2_cache->tail] = *(ch_data->ch4_data + data_index);

                L2_cache->tail++;//Loop save to L2 cache
                if(L2_cache->tail >= L2_cache->buffer_size)
                {
                    L2_cache->tail = 0;
                    qDebug() << "L2 cache update loop";
                }

                if(trigger.auto_flag)
                {
                    if(trigger.check_trigger_flag && L2_cache->fetch_finish)
                    {
                        if(L2_cache->auto_trigger_count > trigger.auto_trigger_gap)
                        {
                            //to set frame form current location - 1000 to current loaction whether time offset
                            L2_cache->auto_trigger_count = 0;
                            long frame_head_temp = L2_cache->tail - 1000 * oversample_rate;
                            L2_cache->frame_head = frame_head_temp > 0 ? frame_head_temp :
                                                                         frame_head_temp + L2_cache->buffer_size;
                            L2_cache->frame_tail = L2_cache->tail;
                            L2_cache->fetch_finish = false;
                            L2_cache->show_valid = true;
                        }
                        L2_cache->auto_trigger_count++;
                    }
                    else
                        L2_cache->auto_trigger_count = 0;
                }

                if(!L2_cache->show_valid) // to check whether finish receive
                {
                    if(L2_cache->frame_tail == L2_cache->tail)
                    {
                        L2_cache->show_valid = true;
                        L2_cache->fetch_finish = false;
                    }
                }

                if(!trigger.check_trigger_flag) // to open next trigger
                {
                    if(L2_cache->tail == L2_cache->next_trigger_check_location)
                    {
                        trigger.check_trigger_flag = true;
                    }
                }
            }// update L2 cache
        }//Loop raw buffer
    }
}

void oscilloscope::timeDragRes(int diff_value)
{
    int time_offset_temp = time_setting.time_offset + diff_value;
    ui->scopeview->changeTimeLocation(time_offset_temp);
}

void oscilloscope::timeDragFinishRes()
{
//To do
}

void oscilloscope::view_update()
{
    if(L2_cache->fetch_finish == true) // no new data form L2 cache
        return;

    QList<QVector<int16_t> > disply_data;
    uint8_t updata_mask = 0;

    CH1_point_buffer.clear();
    CH2_point_buffer.clear();
    CH3_point_buffer.clear();
    CH4_point_buffer.clear();

    qDebug()<<"Tl,tv:" << L2_cache->trigger_location << L2_cache->CH_buffer[0][L2_cache->trigger_location];

    for(int i = 0; i < 1000; i++)
    {
        int index = (L2_cache->frame_head + static_cast<int>(i * oversample_rate)) % L2_cache->buffer_size;
        if(i == 0) qDebug()<< "first index:"<< index;
        if(i == 999) qDebug()<< "last index:"<< index;
        if(CH1_setting.channel_on)
        {
            CH1_point_buffer << L2_cache->CH_buffer[0][index] * CH1_setting.channel_scale;
        }
        if(CH2_setting.channel_on)
        {
            CH2_point_buffer << L2_cache->CH_buffer[1][index] * CH2_setting.channel_scale;
        }
        if(CH3_setting.channel_on)
        {
            CH3_point_buffer << L2_cache->CH_buffer[2][index] * CH3_setting.channel_scale;
        }
        if(CH4_setting.channel_on)
        {
            CH4_point_buffer << L2_cache->CH_buffer[3][index] * CH4_setting.channel_scale;
        }
    }


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

    ///////////////////////////////////////For test
    scopeview->setBottomLabel(1, QString("Data gap: %1").arg(datagap));
    scopeview->setBottomLabel(2, QString("oversample: %1").arg(oversample_rate));
    scopeview->setBottomLabel(3, QString("Trigger level: %1").arg(trigger.trigger_level));

    L2_cache->fetch_finish = true;
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
    time_setting.time_scale_code = 0;
    time_setting.time_scale = 1;
    ui->timescaleENC->resetValue();
    update_time_parament();
}

void oscilloscope::timeOffsetReset()
{
    time_setting.time_offset = 0;
    ui->timeoffsetENC->resetValue();
    scopeview->changeTimeLocation(time_setting.time_offset);
    update_time_parament();
}

void oscilloscope::triggerReset()
{
    trigger_setting.trigger_level = 0;
    ui->triggerENC->resetValue();
    update_trigger_location();
}

void oscilloscope::CH1OffsetReset()
{
    CH1_setting.channel_offset = 0;
    ui->CH1offsetENC->resetValue();    
    update_trigger_location();
}

void oscilloscope::CH2OffsetReset()
{
    CH2_setting.channel_offset = 0;
    ui->CH2offsetENC->resetValue();    
    update_trigger_location();
}

void oscilloscope::CH3OffsetReset()
{
    CH3_setting.channel_offset = 0;
    ui->CH3offsetENC->resetValue();    
    update_trigger_location();
}

void oscilloscope::CH4OffsetReset()
{
    CH4_setting.channel_offset = 0;
    ui->CH4offsetENC->resetValue();    
    update_trigger_location();
}

void oscilloscope::CH1ScaleReset()
{
    CH1_setting.channel_scale_code = 0;
    CH1_setting.channel_scale = 1;
    ui->CH1scaleENC->resetValue();    
    update_trigger_location();
}

void oscilloscope::CH2ScaleReset()
{
    CH2_setting.channel_scale_code = 0;
    CH2_setting.channel_scale = 1;
    ui->CH1scaleENC->resetValue();    
    update_trigger_location();
}

void oscilloscope::CH3ScaleReset()
{
    CH3_setting.channel_scale_code = 0;
    CH3_setting.channel_scale = 1;
    ui->CH1scaleENC->resetValue();
    update_trigger_location();
}

void oscilloscope::CH4ScaleReset()
{
    CH4_setting.channel_scale_code = 0;
    CH4_setting.channel_scale = 1;
    ui->CH1scaleENC->resetValue();
    update_trigger_location();
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
    update_time_parament();
}

void oscilloscope::timeOffsetSet(int value)
{
    time_setting.time_offset += value;
    //To test
    scopeview->changeTimeLocation(time_setting.time_offset);
}

void oscilloscope::triggerSet(int value)
{
    trigger_setting.trigger_level += value;
    update_trigger_location();
}

void oscilloscope::CH1OffsetSet(int value)
{
    CH1_setting.channel_offset += value;
    scopeview->changeCHOffset(1,CH1_setting.channel_offset);
    update_trigger_location();
}

void oscilloscope::CH2OffsetSet(int value)
{
    CH2_setting.channel_offset += value;
    scopeview->changeCHOffset(2,CH2_setting.channel_offset);
    update_trigger_location();
}

void oscilloscope::CH3OffsetSet(int value)
{
    CH2_setting.channel_offset += value;
    scopeview->changeCHOffset(3,CH3_setting.channel_offset);
    update_trigger_location();
}

void oscilloscope::CH4OffsetSet(int value)
{
    CH4_setting.channel_offset += value;
    scopeview->changeCHOffset(4,CH4_setting.channel_offset);
    update_trigger_location();
}

void oscilloscope::CH1ScaleSet(int value)
{
    CH1_setting.channel_scale_code += value;
    if(CH1_setting.channel_scale_code > 100)
        CH1_setting.channel_scale_code = 100;
    if(CH1_setting.channel_scale_code < -100)
        CH1_setting.channel_scale_code = -100;
    CH1_setting.channel_scale = level_trans(CH1_setting.channel_scale_code);
    update_trigger_location();
}

void oscilloscope::CH2ScaleSet(int value)
{
    CH2_setting.channel_scale_code += value;
    if(CH2_setting.channel_scale_code > 100)
        CH2_setting.channel_scale_code = 100;
    if(CH2_setting.channel_scale_code < -100)
        CH2_setting.channel_scale_code = -100;
    CH2_setting.channel_scale = level_trans(CH2_setting.channel_scale_code);
    update_trigger_location();
}

void oscilloscope::CH3ScaleSet(int value)
{
    CH3_setting.channel_scale_code += value;
    if(CH3_setting.channel_scale_code > 100)
        CH3_setting.channel_scale_code = 100;
    if(CH3_setting.channel_scale_code < -100)
        CH3_setting.channel_scale_code = -100;
    CH3_setting.channel_scale = level_trans(CH3_setting.channel_scale_code);
    update_trigger_location();
}

void oscilloscope::CH4ScaleSet(int value)
{
    CH4_setting.channel_scale_code += value;
    if(CH4_setting.channel_scale_code > 100)
        CH4_setting.channel_scale_code = 100;
    if(CH4_setting.channel_scale_code < -100)
        CH4_setting.channel_scale_code = -100;
    CH4_setting.channel_scale = level_trans(CH4_setting.channel_scale_code);
    update_trigger_location();
}

bool oscilloscope::check_trigger(size_t index)
{
    int16_t current_value = *(trigger.trigger_data + index);
    if(current_value >= trigger.trigger_level) //if current data is higher than trigger level
    {
        if(trigger.above_level_flag) // if previous data is higher than trigger level
        {
            return false;
        }
        else // if previous data is lower than trigger leve
        {
            trigger.above_level_flag = true;
            if(trigger.rise_flag)
            {
                return true;
            }
        }
    }
    else //if current data is lower than trigger level
    {
        if(trigger.above_level_flag) // if previous data is higher than trigger level
        {
            trigger.above_level_flag = false;
            if(trigger.fall_flag)
            {
                return true;
            }
        }
        else // if previous data is lower than trigger leve
        {
            return false;
        }
    }
    return false;
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

void oscilloscope::update_trigger_location()
{
    int trigger_level_show;
    channel_setting_t *trigger_ch;
    switch(trigger_setting.trigger_ch)
    {
    case CH1: trigger_ch = &CH1_setting; break;
    case CH2: trigger_ch = &CH2_setting; break;
    case CH3: trigger_ch = &CH3_setting; break;
    case CH4: trigger_ch = &CH4_setting; break;
    default: break;
    }
    trigger_level_show = trigger_setting.trigger_level * trigger_ch->channel_scale + trigger_ch->channel_offset;
    scopeview->changeTriggerLocation(trigger_level_show);
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

void oscilloscope::update_time_parament()
{
    datagap = time_setting.time_scale;
    if(datagap < 10)
    {
        oversample_rate = datagap;
        datagap = 1.0f;
    }
    else
    {
        oversample_rate = 10;
        datagap = datagap / 10;
    }
}

void oscilloscope::update_trigger_parament(scope_ch_data_t *ch_data)
{
    trigger.trigger_level = trigger_setting.trigger_level;
    trigger.auto_flag = trigger_setting.trigger_mode == mode_auto ? true : false;
    if(trigger_setting.trigger_direct == direrc_fall || trigger_setting.trigger_direct == direrc_risefall)
        trigger.fall_flag = true;
    else
        trigger.fall_flag = false;

    if(trigger_setting.trigger_direct == direrc_rise || trigger_setting.trigger_direct == direrc_risefall)
        trigger.rise_flag = true;
    else
        trigger.rise_flag = false;

    trigger.trigger_gap = 1000 * oversample_rate;
    switch (static_cast<int>(trigger_setting.trigger_ch)) {
    case CH1: trigger.trigger_data = ch_data->ch1_data; break;
    case CH2: trigger.trigger_data = ch_data->ch2_data; break;
    case CH3: trigger.trigger_data = ch_data->ch3_data; break;
    case CH4: trigger.trigger_data = ch_data->ch4_data; break;
    default: break;
    }
}


void oscilloscope::on_singleBTN_clicked()
{
    if(run_status == status_stop)
        run_status = status_run;
    single_trigger_flag = true;
}

void oscilloscope::on_runBTN_clicked()
{
    if (run_status == status_run)
        run_status = status_stop;
    else
        run_status = status_run;
    single_trigger_flag = false;
}
