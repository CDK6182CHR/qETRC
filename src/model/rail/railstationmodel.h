#pragma once

#include <memory>

#include "data/common/direction.h"
#include "model/general/qemoveablemodel.h"

class RailStation;
class Railway;
/**
 * @brief The RailStationModel class
 * 线路的站表（里程表）的数据模型，用于编辑里程的表格。
 * 注意使用StandartItem来暂存数据。
 * 注意要求parent()是QWidget，以方便弹出警告对话框。
 */
class RailStationModel : public QEMoveableModel
{
    Q_OBJECT
    std::shared_ptr<Railway> railway;
    const bool commitInPlace;
public:
    enum Columns{
        ColName=0,
        ColMile,
        ColCounter,
        ColLevel,
        ColShow,
        ColDir,
        ColSingle,
        ColPassenger,
        ColFreight,
        ColMAX
    };
    explicit RailStationModel(bool inplace, QWidget *parent = nullptr);

    explicit RailStationModel(std::shared_ptr<Railway> rail,bool inplace,
                              QWidget* parent=nullptr);

    void setRailway(std::shared_ptr<Railway> rail);

    /**
     * @brief setRailwayForDir
     * 为标尺排图向导等处提供的模式，按顺序列出上行或下行经过的所有站
     */
    void setRailwayForDir(std::shared_ptr<Railway> rail, Direction dir);

    virtual void setupNewRow(int row)override;

    /*
     * 默认实现的DisplayRole和EditRole是同一个东西。要想显示的不一样，只有重写
     * See: https://doc.qt.io/qt-5/qstandarditem.html#data
     */
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /**
     * 三个Check的列不能编辑只能Check
     */
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

    /**
    * 提交前检查表格数据是否存在非法情况，如果存在则以对话框提示。
    * 返回检查是否通过。
    */
    bool checkRailway(std::shared_ptr<Railway> rail);

    /**
     * 按当前数据生成Railway对象。新对象只包含线路基础数据。
     * 可能会出错（例如，站名重复），此时返回空。
     */
    std::shared_ptr<Railway> generateRailway()const;

    /**
     * 站名列保存的站点指针信息。
     * 如果表格不是只读的（i.e. 允许新增行），则返回不一定安全。
     */
    std::shared_ptr<const RailStation> getRowStation(int row);

    /**
     * 修改并报告结果（是否成功）。
     * 不是slot。seealso `actApply`
     */
    bool applyChange();

signals:

    /**
     * 将实际处理的权限交给RailContext
     */
    void actStationTableChanged(std::shared_ptr<Railway> railway,std::shared_ptr<Railway> newtable,
        bool equiv);

    void dataSubmitted();

public slots:

    /**
     * 提交数据，即点击了确定
     * 注意不能用submit...这个会被自动Call
     * seealso `applyChange()`
     */
    void actApply();

    /**
     * 撤销数据更改，即点击还原
     */
    void actCancel();

    inline void refreshData() { setupModel(); }

    virtual bool submit()override;


private:
    void setupModel();

    void setStationRow(int row,std::shared_ptr<const RailStation> st);
};





