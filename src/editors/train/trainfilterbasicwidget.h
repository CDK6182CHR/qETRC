#pragma once

#include <QWidget>

#include <util/buttongroup.hpp>

class SelectTrainTypeListWidget;
class TrainFilterCore;
class SelectRoutingListWidget;
class TrainNameRegexTable;
class StationNameRegexTable;
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
    QCheckBox* ckStarting, * ckTerminal;
    RadioButtonGroup<3> *gpPassen;

    SelectTrainTypeListWidget* lstType=nullptr;
    TrainNameRegexTable* tabInclude=nullptr,*tabExclude=nullptr;
    StationNameRegexTable* tabStarting = nullptr, * tabTerminal = nullptr;
    SelectRoutingListWidget* lstRouting=nullptr;
public:
    explicit TrainFilterBasicWidget(TrainCollection& coll, TrainFilterCore* core, QWidget *parent = nullptr);
    auto* core(){return _core;}
    void setCore(TrainFilterCore* core){_core=core; refreshData();}
    void setCoreSimple(TrainFilterCore* core) { _core = core; }

    /**
     * @brief clearFilter
     * 2023.01.21
     * @returns whether does cleared; if rejected by user, return false.
     */
    bool clearFilter();

    /**
     * 2023.01.21  Set current data to the given object.
     * This is mainly used for Predef version.
     */
    void appliedData(TrainFilterCore* core);

private:
    void initUI();
    
public slots:
    /**
     * This is not internally called. 
     * Set the data into corresponding core object.
     */
    void actApply();
    void clearNotChecked();
    void refreshData();
    void refreshDataWith(const TrainFilterCore* core);
};

