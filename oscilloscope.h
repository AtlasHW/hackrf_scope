#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <QDialog>

#include <QGraphicsScene>
#include <QGraphicsItem>

#include <QTimer>

#include "scopeview.h"

typedef struct{
    int ch_count;
    int16_t *ch1_data;
    int16_t *ch2_data;
    int16_t *ch3_data;
    int16_t *ch4_data;
    size_t buffer_size;
    size_t update_index;
    size_t pre_index;
    size_t update_count;
} scope_ch_data_t;

typedef enum{
    CH1 = 0,
    CH2 = 1,
    CH3 = 2,
    CH4 = 3
}ch_t;

typedef enum{
    direrc_rise       = 0,
    direrc_fall       = 1,
    direrc_risefall   = 2
} trigger_direct_t;

typedef enum{
    mode_normal = 0,
    mode_auto   = 1,
} trigger_mode_t;

typedef enum{
    status_run  = 0,
    status_stop = 1
} run_status_t;

typedef struct{
    float time_scale  = 1;
    int time_scale_code = 0;
    int time_offset = 0;
} time_setting_t;

typedef struct{
    int trigger_level               = 12;
    ch_t trigger_ch                 = CH1;
    trigger_direct_t trigger_direct = direrc_rise;
    trigger_mode_t trigger_mode     = mode_auto;
} trigger_setting_t;

typedef struct{
    int channel_on;
    float channel_scale;
    int channel_offset;
    int channel_scale_code;
} channel_setting_t;


namespace Ui {
class oscilloscope;
}

class oscilloscope : public QDialog
{
    Q_OBJECT

public:
    explicit oscilloscope(QWidget *parent = 0);
    ~oscilloscope();

public slots:
    //To receive up layer signal
    void rx_data_update(scope_ch_data_t *ch_data);

    //To receive down layer signal
    void timeDragRes(int diff_value);
    void timeDragFinishRes();

private slots:
    void view_update();

    void CH1_setEnable(bool value);
    void CH2_setEnable(bool value);
    void CH3_setEnable(bool value);
    void CH4_setEnable(bool value);

    void triggerDirectSet(int direct);
    void triggerModeSet(int mode);
    void triggerCHSet(int CH);

    void timeScaleReset();
    void timeOffsetReset();
    void triggerReset();
    void CH1OffsetReset();
    void CH2OffsetReset();
    void CH3OffsetReset();
    void CH4OffsetReset();
    void CH1ScaleReset();
    void CH2ScaleReset();
    void CH3ScaleReset();
    void CH4ScaleReset();

    void timeScaleSet(int value);
    void timeOffsetSet(int value);
    void triggerSet(int value);
    void CH1OffsetSet(int value);
    void CH2OffsetSet(int value);
    void CH3OffsetSet(int value);
    void CH4OffsetSet(int value);
    void CH1ScaleSet(int value);
    void CH2ScaleSet(int value);
    void CH3ScaleSet(int value);
    void CH4ScaleSet(int value);

private://veriate
    Ui::oscilloscope *ui;

    ScopeView *scopeview;

    int time_offset;

    QVector<int> CH1_point_buffer;
    QVector<int> CH2_point_buffer;
    QVector<int> CH3_point_buffer;
    QVector<int> CH4_point_buffer;

    //Timer for scope image update
    QTimer *view_update_timer;

    time_setting_t time_setting;
    trigger_setting_t trigger_setting;
    channel_setting_t CH1_setting;
    channel_setting_t CH2_setting;
    channel_setting_t CH3_setting;
    channel_setting_t CH4_setting;

    run_status_t run_status;

    size_t rec_count;

    size_t trigger_location;

private://funtion

    int check_trigger(scope_ch_data_t *scope_ch_data,size_t *trigger_location);

    void update_trigger_label();

    float time_trans(int value);
    float level_trans(int value);

};

#endif // OSCILLOSCOPE_H
