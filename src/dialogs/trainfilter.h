#pragma once

#include <QDialog>
#include "util/buttongroup.hpp"
#include "data/train/trainfiltercore.h"

class SelectRoutingDialog;
class TrainNameRegexDialog;
class SelectTrainTypeDialog;
class QCheckBox;
class Diagram;
/**
 * @brief The TrainFilter class
 * pyETRC.TrainFilter  功能类似，逻辑基本照搬，但直接继承QDialog
 */
class TrainFilter : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    TrainFilterCore core;

    QCheckBox* ckType,*ckInclude,*ckExclude,*ckRouting;
    QCheckBox* ckShowOnly,*ckInverse;
    RadioButtonGroup<3> *gpPassen;

    SelectTrainTypeDialog* dlgType=nullptr;
    TrainNameRegexDialog* dlgInclude=nullptr,*dlgExclude=nullptr;
    SelectRoutingDialog* dlgRouting=nullptr;

public:
    TrainFilter(Diagram& diagram_, QWidget* parent=nullptr);
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
    void filterApplied(TrainFilter* s);
private slots:
    void selectType();
    void setInclude();
    void setExclude();
    void selectRouting();
    void actApply();
public slots:
    void clearFilter();
};

