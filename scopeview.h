#ifndef SCOPEVIEW_H
#define SCOPEVIEW_H

#include <QWidget>

typedef struct{
    uint8_t CH_EN;
    int time_offset;
    int trigger_location;
    int CH1_offset;
    int CH2_offset;
    int CH3_offset;
    int CH4_offset;

    uint8_t trigger_direct;

    QString T_solution;
    QString trigger_label;
    QString CH1_solution;
    QString CH2_solution;
    QString CH3_solution;
    QString CH4_solution;
} scope_para_t;

class ScopeView : public QWidget
{
    Q_OBJECT
public:
    explicit ScopeView(QWidget *parent = nullptr);

    void paramentInit(scope_para_t *para);
    void paramentDefault();
    void createDemoWave(); //Create demo wave for learning

    void enableChannel(int channel);
    void disableChannel(int channel);

    void setCH_Enable(int channel, bool ena);

    void setTimeSolution(QString solution);

    void setTriggerDirection(uint8_t direction);
    void setTriggerLabel(QString lable);

    void setCHLabel(int CH, QString label);

    //Set bottom label: because the mothed need to update widget,
    //the 'setBottomLabelBuffer' is recommanded to use
    void setBottomLabel(int index,QString label);

    //The function will not update widget, the result show in next data update
    void setBottomLabelBuffer(int index,QString label);

    void setTimeLineShow(const bool value);
    void setTriggerLineShow(const bool value);
    void setCHLineShow(uint8_t value);

public slots:
    void changeTimeLocation(int location);
    void changeTriggerLocation(int location);
    void changeCHOffset(int channel, int offset);
    void changeData(uint8_t changeMask, QList<QVector<int16_t> > data);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    int wave_area_W; //Wave display window pixels count of width
    int wave_area_H; //Wave display window pixels count of hight

    uint8_t CH_EN;//bit 0: CH1, bit 1: CH2 ... 1 enable, 0 disable
    uint8_t trigger_direct;//0 rise, 1 fall, 2 rise and fall
    QString CH1_solution;
    QString CH2_solution;
    QString CH3_solution;
    QString CH4_solution;
    QString TIME_solution;
    QString trigger_label;

    int time_line_location;//-500 ~ 500:Normal range; others: hide
    bool time_line_show;

    int trigger_line_location;//-500 ~ 500:Normal range; others: hide
    bool trigger_line_show;

    int CH1_offset;
    int CH2_offset;
    int CH3_offset;
    int CH4_offset;
    uint8_t CH_line_show; //As same as 'CH_EN'

    QVector<int16_t> CH1_draw_buffer;
    QVector<int16_t> CH2_draw_buffer;
    QVector<int16_t> CH3_draw_buffer;
    QVector<int16_t> CH4_draw_buffer;

    QRect Time_drag_rect;
    int time_drag_start_x;
    bool time_drag_press_flag;

    QRect Trigger_drag_rect;
    int trigger_drag_start_y;
    bool trigger_drag_press_flag;

    QRect CH1_drag_rect;
    QRect CH2_drag_rect;
    QRect CH3_drag_rect;
    QRect CH4_drag_rect;
    int CH1_drag_start_y;
    int CH2_drag_start_y;
    int CH3_drag_start_y;
    int CH4_drag_start_y;
    bool CH1_drag_press_flag;
    bool CH2_drag_press_flag;
    bool CH3_drag_press_flag;
    bool CH4_drag_press_flag;

    QString bottom_label1;
    QString bottom_label2;
    QString bottom_label3;
    QString bottom_label4;

private: //Private function
    void setTime_drag_rect(const int location);
    void setTrigger_drag_rect(const int location);
    void setCH1_drag_rect(const int offset);
    void setCH2_drag_rect(const int offset);
    void setCH3_drag_rect(const int offset);
    void setCH4_drag_rect(const int offset);

signals:
    void timeDrag(int);
    void triggerDrag(int);
    void CH1_Drag(int);
    void CH2_Drag(int);
    void CH3_Drag(int);
    void CH4_Drag(int);

    void timeDragFinish();
    void triggerDragFinish();
    void CH1_DragFinish();
    void CH2_DragFinish();
    void CH3_DragFinish();
    void CH4_DragFinish();

};

#endif // SCOPEVIEW_H
