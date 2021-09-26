#pragma once

#include <QSplitter>
#include <railnet/graph/railnet.h>

class QTableView;
class QLineEdit;
class GraphPathModel;
class AdjacentListModel;
class PathOperationModel;
/**
 * @brief The PathSelectWidget class
 * 经由选择器页面。现在作为Wizard的核心页面。
 */
class PathSelectWidget : public QSplitter
{
    Q_OBJECT
    const RailNet& net;
    PathOperationModel* const seqModel;
    AdjacentListModel* const adjModel;
    GraphPathModel* const pathModel;

    QLineEdit* edStation,*edRailName;
    QTableView* seqTable,*adjTable,*pathTable;
public:
    PathSelectWidget(const RailNet& net, QWidget* parent=nullptr);
    auto* getSeqModel(){return seqModel;}
private:
    void initUI();

    // 初始化table的重复代码..
    QTableView* setupTable();
private slots:

    /**
     * 增删了路径  更新邻接站表
     */
    void onSelectSeqChanged(std::shared_ptr<const RailNet::vertex> v);

    /**
     * 邻站表的当前行变化，去更新邻线表
     */
    void onCurrentAdjStationChanged(const QModelIndex& idx);

    /**
     * 强制触发更新Path。
     */
    void setAdjStationRow(int row);

    void addByShortestPath();
    void popSelect();
    void clearSelect();

    /**
     * 双击调用
     */
    void addByAdjStation(const QModelIndex& idx);
    void addByAdjStationBtn();

    /**
     * 双击调用
     */
    void addByAdjRailway(const QModelIndex& idx);
    void addByAdjRailwayBtn();

};

