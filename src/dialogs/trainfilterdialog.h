#pragma once

#include <QDialog>
#include "data/train/trainfiltercore.h"

class PredefTrainFilterCore;
class TrainCollection;
class TrainFilterBasicWidget;
class TrainFilterCombo;
/**
 * @brief The TrainFilterDialog class
 * 2023.01.22  The new version of TrainFilter,
 * including selection of PredefTrainFilter.
 */
class TrainFilterDialog : public QDialog
{
    Q_OBJECT
    TrainCollection& coll;
    TrainFilterCore core;
    TrainFilterBasicWidget* basic;
    TrainFilterCombo* combo;
    friend class TrainFilterSelector;
public:
    TrainFilterDialog(TrainCollection& coll, QWidget* parent=nullptr);
    bool check(std::shared_ptr<const Train> train)const;
    const auto& getCore()const { return core; }

    /**
     * 返回当前筛选器状态下，被选中的车次列表。
     * 线性算法，暂时不引入缓存机制。
     */
    QList<std::shared_ptr<Train>> selectedTrains()const;

private:
    void initUI();
signals:
    void filterApplied(TrainFilterDialog* s);
public slots:
    void clearFilter();
private slots:
    void onComboChanged(const PredefTrainFilterCore* data);
    void actApply();
};

