#pragma once

#include <QStandardItemModel>
#include <memory>
#include <list>
#include "model/delegate/qetimedelegate.h"
#include <QUndoCommand>
#include <map>
#include "data/train/trainstation.h"
#include "data/diagram/stationbinding.h"

class Train;
struct DiagramOptions;

/**
 * @brief The TimetableQuickModel class
 * 双行式的列车时刻表
 */
class TimetableQuickModel : public QStandardItemModel
{
    Q_OBJECT;
	const DiagramOptions& _ops;
protected:
    std::shared_ptr<Train> train;
    bool updating = false;
    std::map<const TrainStation*, TrainStationBoundingList> boundingMap;
public:
    enum {
        ColName,
        ColTime,
        ColNote,
        ColMAX
    };

    explicit TimetableQuickModel(const DiagramOptions& ops, QObject *parent = nullptr);
    auto getTrain(){return train;}

    /**
     * 指定行在勾选了【只显示停车/营业站】时，是否总是显示。
     * 保证所给row是偶数。
     */
    bool isAlwaysShow(int row)const;

    /**
     * 返回指定行的列车绑定情况
     */
    TrainStationBoundingList getBoundingList(int row)const;

    /**
     * 获取指定行所示的车站迭代器。保证入参row为偶数。
     */
    std::list<TrainStation>::iterator trainStationForRow(int row)const;

    TrainTime arriveTimeForRow(int row)const;
    TrainTime departTimeForRow(int row)const;

protected:
    void setTimeItem(int row,const TrainTime& tm);
    void setTimeItem(int row,const TrainTime& tm,const QString& text);

    void setupModel();
    void setStationColor(int row, const QColor& color);

    /**
     * 通过遍历所有的TrainLine，搞一个TrainStation -> BoudingList的映射表
     * 每次设定新的列车时使用
     */
    void setupBoudingMap();

    /**
     * 创建不可编辑的Item
     */
    QStandardItem* NESI(const QString& text);
    QStandardItem* NESI();

    /**
     * 保证所给row是偶数。
     * 配置一个车站的最后一列数据，以及颜色
     */
    void setStationStopCol(const TrainStation& station, int row);

    /**
     * 保证所给row为偶数
     * 设置一个车站的所有内容，除了车站时刻那部分
     */
    void setupStationExceptTime(std::list<TrainStation>::iterator p, int row);


public slots:
    void refreshData();
    void setTrain(std::shared_ptr<Train> train);
};

class QUndoStack;

/**
 * 支持修改的版本。
 * 数据展示规则一样，只是新增了修改的接口
 */
class TimetableQuickEditableModel :public TimetableQuickModel
{
    Q_OBJECT
    QUndoStack* const _undo;
public:
    explicit TimetableQuickEditableModel(const DiagramOptions& ops, QUndoStack* undo_, QObject* parent = nullptr);

private:

    /**
     * 时刻变化后更新页面：时刻显示，停时，颜色
     * 保证所给row为偶数
     */
    void updateStationTime(const TrainStation& station, int row);

signals:
    void trainStationTimeUpdated(std::shared_ptr<Train>, bool repaint);

private slots:

    /**
     * 只考虑TimeDataRole变化的情况；更新指定行的数据。
     */
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
        const QVector<int>& roles);

    /**
     * 指定行的时刻变化：操作压栈，更新时刻表；
     * 重新设置显示。
     */
    void onStationTimeChanged(int row);

public slots:
    /**
     * 执行时刻微调。更改表格数据，发送信号。
     * 注意train不一定是当前表格的列车。如果不是，不用更新，直接通知铺画就行。
     * 外部的后处理：铺画运行图；更改TrainList的数据。
     * 速览信息窗口不主动更新；Context没有需要更新的。
     * @param data cmd里面保存的那个数据对象
     */
    void commitStationTimeChange(std::shared_ptr<Train> train, int row,
        TrainStation& data, bool repaint);
};

namespace qecmd {

    /**
     * 单个车站的时刻表变化。同一车站的连续变化则合并操作。
     * 注意如果此操作前面发生了时刻表提交更改操作，则TrainStation的地址不能保证不变。
     * 但在相同的状态下，行号是一定不变的。因此保存行号。保证行号为偶数。
     */
    class AdjustTrainStationTime : public QUndoCommand
    {
        std::shared_ptr<Train> train;
        int row;
        TrainStation data;
        TimetableQuickEditableModel* const model;
        const bool repaint;
        static constexpr int ID = 101;
    public:
        AdjustTrainStationTime(std::shared_ptr<Train> train_,int row_,
            TrainStation&& data, TimetableQuickEditableModel* const model_,
            bool repaint_,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
        virtual int id()const override { return ID; }
        virtual bool mergeWith(const QUndoCommand* other)override;
    };
}


