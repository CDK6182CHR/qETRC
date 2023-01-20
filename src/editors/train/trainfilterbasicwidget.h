#pragma once

#include <QWidget>

#include <util/buttongroup.hpp>

class SelectTrainTypeListWidget;
class TrainFilterCore;
class SelectRoutingDialog;
class TrainNameRegexDialog;
class QCheckBox;
class TrainCollection;

/**
 * @brief The TrainFilterBasic class
 * 2023.01.15 The part for editing TrainFilter, both predefined and temporary.
 * For classic usage, refreshData() should be called after construction.
 */
class TrainFilterBasicWidget : public QWidget
{
    Q_OBJECT
    TrainCollection& coll;
    TrainFilterCore* _core;

    QCheckBox* ckType,*ckInclude,*ckExclude,*ckRouting;
    QCheckBox* ckShowOnly,*ckInverse;
    RadioButtonGroup<3> *gpPassen;

    SelectTrainTypeListWidget* lstType=nullptr;
    TrainNameRegexDialog* dlgInclude=nullptr,*dlgExclude=nullptr;
    SelectRoutingDialog* dlgRouting=nullptr;
public:
    explicit TrainFilterBasicWidget(TrainCollection& coll, TrainFilterCore* core, QWidget *parent = nullptr);
    auto* core(){return _core;}
    void setCore(TrainFilterCore* core){_core=core; refreshData();}

private:
    void initUI();
private slots:
    void selectType();
    void setInclude();
    void setExclude();
    void selectRouting();
    void actApply();
public slots:
    void clearFilter();
    void refreshData();

};

