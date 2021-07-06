#pragma once

#include <memory>
#include <QDialog>
#include <QStandardItemModel>

#include "data/train/train.h"
#include "data/diagram/diagram.h"

/**
 * @brief The TrainEventModel class
 * 列车事件表的模型。采用只读模型，但是注意采用StandardItemModel保存数据，
 * 因为对话框存续期间，线路列车信息可能变化，导致车站指针悬空。
 */
class TrainEventModel:public QStandardItemModel
{
    Q_OBJECT;
    std::shared_ptr<Train> train;
    Diagram& diagram;

    /**
     * @brief etrcReport
     * ETRC风格的报告，计算后立即生成好
     */
    QString etrcReport;

    enum Columns {
        ColRail=0,
        ColTime,
        ColPlace,
        ColMile,
        ColEvent,
        ColOther,
        ColNote,
        ColMAX
    };

public:
    TrainEventModel(std::shared_ptr<Train> train_,
                    Diagram& diagram_,
                    QObject* parent=nullptr);

    const QString& etrc()const { return etrcReport; }

private:
    void setupModel();
};


/**
 * @brief The TrainEventDialog class
 * 列车事件表对话框
 */
class TrainEventDialog : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    std::shared_ptr<Train> train;
    TrainEventModel* model;
public:
    TrainEventDialog(Diagram& diagram_, std::shared_ptr<Train> train, QWidget* parent = nullptr);

    void initUI();

private slots:
    void exportETRC();
    void exportText();
    void exportExcel();
};

