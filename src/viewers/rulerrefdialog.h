#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include "data/diagram/diagram.h"

/**
 * @brief The RulerRefModel class
 * 标尺对照。适用于单个车次，一次性展示一个Adapter的情况，可以切换。
 * 如果切换Adapter，则reset
 * 应保证所给车次在本运行图至少有一个Adapter，否则Dialog就不构造
 */
class RulerRefModel:
        public QStandardItemModel
{
    Diagram& diagram;
    std::shared_ptr<Train> train;
    std::shared_ptr<TrainAdapter> adp{};
public:
    enum{
        ColInterval=0,
        ColStd,
        ColStart,
        ColStop,
        ColReal,
        ColMile,
        ColSpeed,
        ColAppend,
        ColDiff,
        ColMAX
    };

    RulerRefModel(Diagram& diagram_, std::shared_ptr<Train> train_,
                  QObject* parent=nullptr);
    void setupModel();

    /**
     * @brief setIndex
     * 选择下标为i的TrainAdapter，更新数据
     */
    void setIndex(int i);
};


/**
 * @brief The RulerRefDialog class
 * 标尺对照
 */
class RulerRefDialog : public QDialog
{
    Q_OBJECT
public:
    RulerRefDialog();
};


