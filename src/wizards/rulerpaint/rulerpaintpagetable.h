#pragma once
#include <QWizardPage>
#include <QStandardItemModel>
#include <memory>
#include <utility>

#include "util/buttongroup.hpp"
#include "data/train/train.h"

class RulerPaintPageTable;
class QComboBox;

/**
 * @brief The RulerPaintModel class
 * 排图表格的模型。初始化时为空；设置好Ruler、起始站、方向后，重新setup
 * 不负责管理列车对象；但要负责计算时刻
 * 这里负责初始化好两个comboBox，包含可用的始发站、终到站信息。
 */
class RulerPaintModel: public QStandardItemModel
{
    Q_OBJECT;
    RulerPaintPageTable*const page;
    std::shared_ptr<Railway> railway{};
    std::shared_ptr<Ruler> ruler{};
    Direction dir{};
    std::shared_ptr<const RailStation> anchorStation;
    int _anchorRow=-1;     //参考行
    int startRow=-1,endRow=-1;   //起始和结束的行
    QComboBox*const cbStart;
    QComboBox*const cbEnd;

    bool updating=false;
public:
    enum {
        ColStation = 0,
        ColMinute,
        ColSecond,
        ColArrive,
        ColDepart,
        ColAppend,
        ColAdjust,
        ColInterval,
        ColMAX
    };

    RulerPaintModel(RulerPaintPageTable* page_, QObject* parent=nullptr);

    /**
     * 进入排图页面时初始化。注意不能乱调用！
     * 设置所有站停车时间为0，然后计算时刻。
     * 从Anchor车站开始，向前、向后插入车站，直到没有可用标尺数据。
     * 注意此时总的行数不一定是总站数。
     * 注意：每一行的第0列保存RulerNode数据，除了参考行之外。
     */
    void setupModel(std::shared_ptr<Railway> railway_,
                    std::shared_ptr<Ruler> ruler_,
                    Direction dir_,
                    std::shared_ptr<const RailStation> anchorStation_);

    inline int anchorRow()const{return _anchorRow;}

    auto* getComboStart(){return cbStart;}
    auto* getComboEnd(){return cbEnd;}

private:

    /**
     * @brief initRow
     * 初始化车站的行：填充空白数据
     */
    void initRow(int row, std::shared_ptr<const RailStation> st, 
        std::shared_ptr<const RulerNode> node);

    /**
     * @brief calculateRow
     * 根据相邻行的数据，计算单一行的数据。递推。
     */
    //void calculateRow(int row);

    /**
     * @brief updateFromRow
     * 从指定行（包括）开始，向端点处调整数据。当数据不变时，可以提前结束。
     * 注意若指定行为参考行，则需要递归一次，前后都调整。
     * 从所给行的到达时刻（正推）或出发时刻（反推）开始计算。
     * 但如果所给行是参考行，则认为到达时刻是正确的。
     */
    void updateFromRow(int row);

    int getStopSecs(int row)const;

    inline bool rowInRange(int row)const {
        return row >= 0 && row < rowCount();
    }

    /// <summary>
    /// 计算所给行的附加情况（字符串），和所给行上一站至本站的区间历时。
    /// 区间附加情况直接填表了
    /// 注意prev, cur皆是指循环遍历方向的。
    /// </summary>
    /// <param name="r">当前行号。已知不是anchor</param>
    /// <param name="dr">行数变更方向，1或-1</param>
    /// <param name="prev_stopped">前一站是否停车</param>
    /// <param name="cur_stopped">本站是否停车</param>
    /// <returns>区间运行秒数</returns>
    int calRowAppendInterval(int r, int dr, bool prev_stopped, bool cur_stopped);

    /// <summary>
    /// 完成所给行的interval, append, arrive, depart的计算和填写工作。
    /// </summary>
    /// <param name="r">当前行</param>
    /// <param name="dr">循环行数变更方向 1/-1</param>
    /// <param name="prev_stopped">前一站是否停车</param>
    /// <param name="cur_stopped">本站是否停车</param>
    /// <param name="tm">循环的时间，input/output</param>
    /// <returns>是否能够终止循环</returns>
    bool calRowTime(int r, int dr, bool prev_stopped, bool cur_stopped,
        QTime& tm);

public slots:
    /**
     * @brief onDoubleClicked
     * 双击指定行，排图到这里。目前好像无事可做。
     * 注意：这里暂不考虑始发终到的问题
     */
//    void onDoubleClicked(const QModelIndex& idx);

    /**
     * @brief onStartChanged
     * 始发站变化。由combo调用；双击首先调用combo变化，然后再调用这个。
     * 主要是处理在本线始发的情况：清除旧的始发站，设置本站为始发站。
     */
    void onStartChanged(int i);

    void onEndChanged(int i);

    void onAnchorTimeChanged(const QTime& tm);

    /**
     * @brief onAnchorTypeChanged
     * Anchor作为到达还是出发的变化
     */
    void onAnchorTypeChanged();

private slots:

    /**
     * @brief onDataChangedconst
     * 数据变化，只需要考虑停车时间以及调整时间的变化，只考虑改变单一单元格的情况，
     * 否则只处理左上。从数据变化的行到两边终止站调整。暂不考虑始发终到站问题
     */
    void onDataChanged(const QModelIndex &topLeft,
                       const QModelIndex &bottomRight,
                       const QVector<int> &roles = QVector<int>());

    /**
     * 指定行的停时或者调整时刻变了，因此更新数据
     */
    void onRowEditDataChanged(int row);

    /**
     * 设置起始行，且同时更新combo. 
     * 保证数据有效。
     */
    void setStartRow(int r);
    void setEndRow(int r);
};



class Railway;
class Ruler;
class Train;
class QCheckBox;
class QTimeEdit;
class QComboBox;
class QLineEdit;
class QTableView;

class RulerPaintPageStation;

/**
 * @brief The RulerPaintPageTable class
 * 第3步，排图页面
 * 初始化时建立好页面控件，
 * initial的时候从field中提取必要的数据来初始化。
 * 逻辑：暂定维护一个train对象，来保存铺画的局部数据；
 * 数据的整合、铺画交给Wizard去做  Wizard那边应该持有mw，直接操作排图
 * 现在看来初始化好像不需要任何数据？
 */
class RulerPaintPageTable : public QWizardPage
{
    Q_OBJECT;
    RulerPaintPageStation* const pgStation;
    std::shared_ptr<Railway> railway{};
    std::shared_ptr<Ruler> ruler{};
    int anchorRow=-1;

    /**
     * 用于装载排图临时信息的列车对象，全程仅有一个对象
     */
    const std::shared_ptr<Train> tmptrain;

    QTimeEdit* edAnTime;
    RadioButtonGroup<2>* gpAnType;
    ButtonGroup<3,QHBoxLayout,QCheckBox>* gpChecks;
    QComboBox* cbStart,*cbEnd;
    QLineEdit* edAnName;

    RulerPaintModel* const model;
    QTableView* table;

public:
    RulerPaintPageTable(RulerPaintPageStation* pgStation_, QWidget* parent=nullptr);
    virtual void initializePage() override;

    bool anchorAsArrive()const{
        return gpAnType->get(0)->isChecked();
    }

    bool startAtThis()const;
    bool endAtThis()const;

    /**
     * @brief instaneous
     * 即时模式，如果启用的任何数据更改立即铺画；否则只有在双击或者始发终到改变时才铺画。
     */
    bool instaneous()const;

    QTime anchorTime()const;
private :
    void initUI();
private slots:
    void onDoubleClicked(const QModelIndex& idx);
};

