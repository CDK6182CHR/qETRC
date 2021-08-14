#pragma once
#include <QWizardPage>

#include "data/train/traincollection.h"

#include "model/train/timetablestdmodel.h"
#include "util/buttongroup.hpp"

class SelectTrainCombo;
class QTableView;
class QStackedWidget;
class QLineEdit;

/**
 * @brief The RulerPaintPageStart class
 * 标尺排图向导 起始页
 * qETRC的新增，选择列车方面的信息
 */
class RulerPaintPageStart : public QWizardPage
{
    Q_OBJECT;
    TrainCollection& coll;

    /**
     * @brief train
     * 选择作为排图基准的列车对象，即原车次表里面的
     * 注意不应该修改；后面铺画时，应该在另外的对象上操作
     * 如果是新建车次，则这里设置好车次名称，新建对象
     */
    std::shared_ptr<Train> _train{};

    RadioButtonGroup<4,QVBoxLayout>* gpMode;
    TimetableStdModel* const model;
    QTableView* table;

    QStackedWidget* stack;

    QLineEdit *edName;

    /**
     * @brief 三种模式下的选择车次控件
     */
    SelectTrainCombo* sels[3];

    /**
     * @brief Modify模式下的选中行的起始和结束
     */
    int _startRow=-1,_endRow=-1;

public:
    enum PaintMode{
        NewTrain=0,
        Prepend,
        Append,
        Modify
    };
    RulerPaintPageStart(TrainCollection& coll_, QWidget* parent=nullptr);

    /**
     * @brief validatePage
     * 点击下一步时，生成当前选中的车次、重新铺画范围等信息。
     * 同时检查输入是否符合要求。
     */
    virtual bool validatePage()override;

    inline int startRow()const{return _startRow;}
    inline int endRow()const{return _endRow;}

    PaintMode getMode();

    auto train()const{return _train;}

private:
    void initUI();
    void initWidget0();
    void initWidget1();
    void initWidget2();
    void initWidget3();
private slots:
    /**
     * 任何一个radio变更触发。另外去判断选择的是哪一个。
     */
    void onModeChanged();

    void onWidget3TrainChanged(std::shared_ptr<Train> t);

};
