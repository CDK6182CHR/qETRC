#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include "util/buttongroup.hpp"
#include "data/common/direction.h"
#include "data/common/traintime.h"
#include "data/calculation/greedypaintconfig.h"
#include <set>
#include <map>
#include <QPointer>

class QTableView;
class Train;
class TrainTimeEdit;
class QCheckBox;
class QLineEdit;
class GreedyPainter;
class Diagram;
class Ruler;
class RailStation;
class StationName;
class DiagramWidget;
class Train;
struct AdapterStation;
class PaintStationInfoWidget;
struct DiagramOptions;


class GreedyPaintConfigModel : public QStandardItemModel
{
    Q_OBJECT;
    const DiagramOptions& _ops;
    std::shared_ptr<Ruler> _ruler;
    int _startRow = -1, _endRow = -1, _anchorRow = -1;
    int _availableFirstRow = 0, _availableLastRow = 0;
    Direction _dir = Direction::Down;
public:
    enum {
        ColName=0,
        ColAnchor,
        ColStart,
        ColEnd,
        ColMinute,
        ColSecond,
        ColFix,
        ColTrack,
        ColActualStop,
        ColArrive,
        ColDepart,
        ColMAX
    };
    GreedyPaintConfigModel(const DiagramOptions& ops, QWidget* parent = nullptr);

    auto ruler() { return _ruler; }
    void setRuler(std::shared_ptr<Ruler> ruler);
    int startRow()const;
    int endRow()const;
    int anchorRow()const { return _anchorRow; }

    /**
     * 初始化数据；铺画相关的数据直接留空
     */
    void refreshData();
    void setDir(Direction dir) { _dir = dir; }

    void updateSettledStops(const std::map<std::shared_ptr<const RailStation>, int>& secs);

    /**
     * 铺画完毕后，将数据设置好
     * 注意铺画不一定成功了；所以站数实际上不一定有效。
     * 但满足单向连续限制，所以查找第一站就行了。
     */
    void setTimetable(std::shared_ptr<Train> train);


    std::shared_ptr<const RailStation> startStation()const;
    std::shared_ptr<const RailStation> endStation()const;
    std::shared_ptr<const RailStation> anchorStation()const;

    std::map<std::shared_ptr<const RailStation>, int>
        stopSeconds()const;

    /**
     * Return the fixed stations.
     */
    std::set<const RailStation*> fixedStations()const;

    /**
     * 2025.08.06  NEW API: station configuration data for each station, including previous stopSeconds and fixedStations()
     */
    std::map<std::shared_ptr<const RailStation>, GreedyPaintStationConfigData>
        stationConfigs()const;

    // 2024.02.11: make public
    std::shared_ptr<const RailStation> stationForRow(int row)const;

    // 2024.02.12  similar to rowForStation(const StationName&); linear alg; -1 if not found
    int rowForStation(std::shared_ptr<const RailStation> st);

    // 2024.02.12: make public
    int stopSecsForRow(int row)const;

    // 2024.02.12  add  for info widget
    int actualStopSecsForRow(int row)const;
    TrainTime arriveTimeForRow(int row)const;
    TrainTime departTimeForRow(int row)const;
    bool fixedForRow(int row)const;

private:
    static QStandardItem* makeCheckItem();
    static QStandardItem* makeReadonlyItem();

    void setRowColor(int row, const QColor& color);
    void setRowBackground(int row, const QColor& color);

    int rowForStation(const StationName& name);


signals:
    void startStationChanged(std::shared_ptr<const RailStation>);
    void endStationChanged(std::shared_ptr<const RailStation>);
    void anchorStationChanged(std::shared_ptr<const RailStation>);
    void fixedStationChanged();
    void stopTimeChanged();

private slots:

    void setStartRow(int row);
    void setEndRow(int row);

    /**
     * 设置锚点行号。
     * 锚点行是核心的参考行；向前向后找到有数据的范围。
     */
    void setAnchorRow(int row);

    /**
     * 此版本专用于初始化，不发射信号
     */
    void setAnchorRowNoSignal(int row);

    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
        const QVector<int>& roles = QVector<int>());

    void onStopTimeChanged(int row);

    void unsetAnchorRow(int row);
    void unsetStartRow(int row);
    void unsetEndRow(int row);

public slots:
    void setRowFixed(int row, bool on);
    void setRowStopSeconds(int row, int secs);
};

class QTextBrowser;

class GreedyPaintPagePaint : public QWidget
{
    Q_OBJECT
    Diagram& diagram;
    GreedyPainter& painter;

    QLineEdit* edTrainName;

    //暂不支持高级铺画目标
    //QCheckBox* ckAdv;

    TrainTimeEdit* edAnchorTime;
    RadioButtonGroup<2>* gpDir, * gpAnchorRole;
    QCheckBox* ckStarting, * ckTerminal, * ckInstaneous, * ckTop;
    QTableView* table;
    QTextBrowser* txtOut;

    GreedyPaintConfigModel* const _model;

    std::shared_ptr<const Train> trainRef;
    std::shared_ptr<Train> trainTmp;

    QLineEdit* edStart, * edAnchor, * edEnd;

    std::map<const RailStation*, QPointer<PaintStationInfoWidget>> infoWidgets;

public:
    explicit GreedyPaintPagePaint(Diagram& diagram_,
            GreedyPainter& painter_,
            QWidget *parent = nullptr);
    auto* model() { return _model; }

    auto getTmpTrain() { return trainTmp; }
private:
    void initUI();

    /**
     * 进行铺画计算，生成临时的列车对象
     */
    std::shared_ptr<Train> doPaintTrain();

    /**
     * 开始计算前，重置临时对象
     */
    void resetTmpTrain();

    /**
     * 将临时对象和和painter.train()进行合并
     */
    void mergeTmpTrain();

signals:
    void showStatus(const QString& );
    void trainAdded(std::shared_ptr<Train>);

    void removeTmpTrainLine(const Train& train);
    void paintTmpTrainLine(std::shared_ptr<Train>);

    void actClose();

    void actExit();

private slots:
    void onDirChanged(bool down);

    /**
     * 当铺画条件改变时，生成临时运行线
     */
    void paintTmpTrain();

    void onApply();

    void onStartChanged(std::shared_ptr<const RailStation>);
    void onEndChanged(std::shared_ptr<const RailStation>);
    void onAnchorChanged(std::shared_ptr<const RailStation>);
    void onFixedChanged();

    void onClearTmp();

    void setTopLevel(bool on);

    void actLoadStopTime();

    void actBatchAddStop();

    void updateInfoWidgets();

    void updateInfoWidget(PaintStationInfoWidget* w);

public slots:
    /**
     * 为一组数据初始化时，设定起始站-锚点站-终止站的标签
     */
    void setupStationLabels();

    // 2023.10.04 add   for cleaning up temporary data
    void clearTmpTrainLine();

    void onPaintingPointClicked(DiagramWidget* d, std::shared_ptr<Train> train, AdapterStation* st);

    void clearInfoWidgets();
};

