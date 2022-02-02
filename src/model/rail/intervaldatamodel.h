#pragma once

#include <QStandardItemModel>
#include <memory>

#include "data/rail/railintervaldata.hpp"


/**
 * @brief The IntervalDataModel class
 * 区间数据（标尺、天窗）model的共性部分，包括初始化、上下行之间复制等。
 * 避免引入模版，只保存Railway对象进行流程控制。
 * 注意Railway可能变，因此采用reference_wrapper
 */
class IntervalDataModel : public QStandardItemModel
{
    Q_OBJECT;
    std::shared_ptr<Railway> _railway;
protected:
    bool updating=false;
public:
    explicit IntervalDataModel(std::shared_ptr<Railway> railway, QObject *parent = nullptr);
    auto railway(){return _railway;}
protected:
    void setRailway(std::shared_ptr<Railway> r){_railway=r;}

    /**
     * @brief setupModel
     * 重置整个Model，注意不在构造函数调用。
     * 采用setupRow()模版方法进行具体的初始化工作。
     */
    virtual void setupModel();

    /**
     * @brief setupRow 模版方法，设置一行的数据。
     * 默认的实现：设置第0列为区间名称。
     */
    virtual void setupRow(int row,std::shared_ptr<RailInterval> railint);

    QString intervalString(const RailInterval& railint)const;

    /**
     * @brief copyRowData  复制表格中的行数据
     * 由copyFromDownToUp和copyFromUpToDown调用
     * 默认实现为空白
     */
    virtual void copyRowData(int from,int to);

    /**
     * @brief checkRowInterval
     * 检查指定行是否对应于所给的区间数据，暂定直接通过字符串比较。
     * 如果不符合，输出警告。
     * Precondition: 第0列是区间名称
     */
    bool checkRowInterval(const RailInterval& railint, int row)const;
    bool checkRowInterval(std::shared_ptr<RailInterval> railint,int row)const;


    /**
     * 当线路发生topo-equiv的更新时，对一行的更新操作。由updateRailIntervals()调用
     * 默认实现为仅更新第0列的区间名称
     */
    virtual void updateEquivRailIntRow(int row, std::shared_ptr<RailInterval> railint);

public slots:
    void copyFromDownToUp();

    void copyFromUpToDown();

    /**
     * 当线路的站表变化时，更新区间表。
     * equiv: 更新前后是否topo等价。如果等价，则只更新区间名称；否则直接刷新。
     * railway: 这里放的是
     */
    void updateRailIntervals(std::shared_ptr<Railway> railway, bool equiv);

};

