#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include "data/diagram/traingap.h"

class Railway;
class QCheckBox;
class TrainFilterCore;
class QTableView;
class TrainGapStatModel:public QStandardItemModel
{
    Q_OBJECT
    TrainGapStatistics stat;
public:
    enum {
        ColPos = 0,
        ColType,
        ColValue,
        ColCount,
        ColMAX
    };
    TrainGapStatModel(const TrainGapStatistics& stat_, QObject* parent=nullptr);
    void refreshData();
private:
    void setupModel();
};

/**
 * @brief The TrainGapStatDialog class
 * 列车间隔总结报告对话框，统计各种类型的最小值以及数量。
 */
class TrainGapStatDialog : public QDialog
{
    Q_OBJECT
    TrainGapStatModel* const model;
    QTableView* table;
public:
    TrainGapStatDialog(const TrainGapStatistics& stat, QWidget* parent=nullptr);
    auto* getModel(){return model;}
    void refreshData();
private:
    void initUI();
};

class Diagram;
class TrainFilter;

/**
 * 列车间隔的全局统计：
 * 每个站的站前、站后各占一行，每一列是一类事件。
 */
class TrainGapSummaryModel :
    public QStandardItemModel
{
    Q_OBJECT;
    friend class TrainGapSummaryDialog;
    Diagram& diagram;
    const std::shared_ptr<Railway> railway;
    const std::map<std::shared_ptr<RailStation>, RailStationEventList> events;
    const TrainFilterCore& filter;

    std::map<TrainGapTypePair, int> globalMin;
    std::map<TrainGapTypePair, int> typeCols;
    std::map<std::shared_ptr<RailStation>, std::map<TrainGapTypePair, int>> localMin;
    int nextCol;

    bool useSingle = false;
    int  cutSecs = 0;
public:
    enum {
        ColStation=0,
        ColOTHERS
    };

    TrainGapSummaryModel(Diagram& diagram_, std::shared_ptr<Railway> railway_, 
        TrainFilter* filter,
        QObject* parent = nullptr);
    void refreshData();
    void setUseSingle(bool on) { useSingle = on; }
    auto getRailway() { return railway; }
    std::shared_ptr<RailStation> stationForRow(int row);
    const auto& getEvents()const { return events; }
    auto& getDiagram() { return diagram; }
    void setCutSecs(int s) { cutSecs = s; }
private:
    // 只做常规的设置数据的事情
    void setupModel();

    // 统计各种出现的类型
    void setupHeader();

    void setupRow(int row, const QString& text,
        const std::map<TrainGapTypePair, int>& val);
};

class TimeIntervalDelegate;
class QSpinBox;

class TrainGapSummaryDialog : public QDialog
{
    Q_OBJECT

    TrainFilter*const filter;   // filter必须在model之前初始化
    TrainGapSummaryModel* const model;

    QCheckBox* ckSingle;
    QTableView* table;
    TimeIntervalDelegate* dele;
    QSpinBox* spCut;
public:
    TrainGapSummaryDialog(Diagram& diagram, std::shared_ptr<Railway> railway,
                          QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void locateOnEvent(int pageIndex, std::shared_ptr<const Railway>,
        std::shared_ptr<const RailStation>, const QTime&);  // 只用来转发
private slots:
    void toCsv();
    void onDoubleClicked(const QModelIndex& idx);
public slots:
    void refreshData();
};

