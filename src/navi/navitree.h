#pragma once

#ifndef QETRC_MOBILE_2

#include <QUndoCommand>
#include <QWidget>

class Forbid;
class Routing;
class Ruler;
class Railway;
class Train;
class DiagramPage;
class TrainPath;
class QUndoStack;
class DiagramNaviModel;
class QMenu;
class PathRuler;

//#include <QMenu>
//#include <QPoint>
//#include <QUndoStack>
//#include <QUndoCommand>
//#include <QVector>

//#include "data/diagram/diagrampage.h"
//#include "model/diagram/diagramnavimodel.h"

class QTreeView;
namespace navi {
    class AbstractComponentItem;
    class DiagramItem;
}

/**
 * @brief The NaviTree class  系统导航那个TreeView
 * 注意这个类暂定只有主程序才用 （i.e. 不给数据库预留），因此持有主程序的undoStack指针
 */
class NaviTree : public QWidget
{
    Q_OBJECT;

    QMenu* mePageList, * meRailList, * meTrainList, * mePage, * meTrain, * meRailway;
    QMenu* meRuler, * meForbid, * meRouting, * meRoutingList;
    QMenu* mePath, * mePathList, * mePathRuler;
    DiagramNaviModel* _model;
    QUndoStack* const _undo;
    QTreeView* tree;
public:
    NaviTree(DiagramNaviModel* model_, QUndoStack* undo, QWidget* parent = nullptr);

    DiagramNaviModel* naviModel() { return _model; }
  

private:
    void initUI();
    void initContextMenus();
    using ACI = navi::AbstractComponentItem;
    ACI* getItem(const QModelIndex& idx);


signals:
    void pageInserted(std::shared_ptr<DiagramPage> page, int index);
    void pageRemoved(int index);

    void focusInPage(std::shared_ptr<DiagramPage> page);
    void focusOutPage();
    void focusInTrain(std::shared_ptr<Train> train);
    void focusOutTrain();
    void focusInRailway(std::shared_ptr<Railway>);
    void focusInRuler(std::shared_ptr<Ruler>);
    void focusInRouting(std::shared_ptr<Routing>);
    void editRailway(std::shared_ptr<Railway>);
    void editTimetable(std::shared_ptr<Train>);
    void editTrain(std::shared_ptr<Train>);
    void editRuler(std::shared_ptr<Ruler>);
    void mergeRuler(std::shared_ptr<Ruler>);
    void editForbid(std::shared_ptr<Forbid>, std::shared_ptr<Railway>);
    void editRouting(std::shared_ptr<Routing>);
    void activatePageAt(int i);
    void focusOutRailway();
    void railwayListChanged();   //只是通知主窗口更新数据  其实都不见得有必要
    void focusInPath(TrainPath*);

    /**
     * 转发给主窗口去画图  所有页面重新铺画
     */
    void trainsImported();
    void removeRulerNavi(std::shared_ptr<Ruler>);
    void removeRoutingNavi(int row);

    void actAddRouting();
    void actBatchParseRouting();
    void actBatchDetectRouting();

    void onEditRailwayFromPageContext();
    void onSwitchToRailwayFromPageContext();

    void actRemoveRailwayAt(int index);

    /**
     * 创建标尺副本比较特殊，发送给RulerContext处理
     */
    void dulplicateRuler(std::shared_ptr<Ruler>);

    void actAddPath();

    void editPathNavi(int idx);

    void removePathNavi(int idx);

    void editPathRulerNavi(std::shared_ptr<PathRuler> pathRuler);

    void removePathRulerNavi(std::shared_ptr<PathRuler> pathRuler);

    void duplicatePathRulerNavi(std::shared_ptr<PathRuler> pathRuler);

    void duplicatePathNavi(int idx);

    /**
     * 2023.08.16  the processing of qecmd::RemoveSingleTrain is passed to TrainContext
     */
    void removeTrainNavi(int idx);
    
    /**
     * 2023.08.22  ask the RailContext to process this en-stack operation.
     */
    void importRailways(QList<std::shared_ptr<Railway>>&);

    void exportForbidToCsv(std::shared_ptr<Forbid>);
    void importForbidFromCsv(std::shared_ptr<Forbid>);

private slots:
    void showContextMenu(const QPoint& pos);
    
    void onDoubleClicked(const QModelIndex& index);

    void onRemovePageContext();

    void onRemoveSingleTrainContext();

    void onRemoveRailwayContext();

    void onDulplicateRailwayContext();

    void onDulplicateTrainContext();

    void onDulplicateRulerContext();

    void onDulplicatePageContext();

    void onRemoveRulerContext();

    void onRemoveRoutingContext();

    void onEditRailwayContext();

    void onEditTrainContext();

    void onEditTimetableContext();

    void onEditRulerContext();

    void onMergeRulerContext();

    void onEditForbidContext();

    void onExportForbidCsvContext();

    void onImportForbidCsvContext();

    void onEditRoutingContext();

    void onActivatePageContext();

    void onCreatePageByRailContext();

    void onSwitchToTrainRoutingContext();

    void onEditPathContext();

    void onRemovePathContext();

    void onDuplicatePathContext();

    void onEditPathRuler();

    void onRemovePathRuler();

    void onDuplicatePathRuler();

    void onCurrentChanged(const QModelIndex& cur, const QModelIndex& prev);

    void actExpand();

    void actCollapse();

    
public slots:
    void actAddRailway();
    void actAddTrain();

    /**
     * 2021.08.13  与标尺排图向导连接
     * 采用标尺排图的方式添加新列车，压栈
     */
    void actAddPaintedTrain(std::shared_ptr<Train> train);

    /**
     * 批量复制车次，操作压栈
     */
    void actBatchAddTrains(const QVector<std::shared_ptr<Train>>& trains);

    void actImportRailways();
    void importRailwayFromDB(std::shared_ptr<Railway> railway);
    void addNewPage();

    /**
     * AddPageDialog返回后进入 操作压栈
     */
    void addNewPageApply(std::shared_ptr<DiagramPage> page);
    
    void importTrains();

    void commitAddPage(std::shared_ptr<DiagramPage> page);

    /**
     * 撤销添加页面，则直接删掉最后一个就好了
     */
    void undoAddPage();

    /**
     * 压栈cmd操作
     */
    void removePage(int index);

    /**
     * 压栈cmd
     */
    void removeSingleTrain(int index);

    void commitRemovePage(int index);

    void undoRemovePage(std::shared_ptr<DiagramPage> page, int index);

    /**
     * 2021.12.18
     * 创建线路副本。这里要求输入名称，然后压栈。
     */
    void actDulplicateRailway(std::shared_ptr<Railway> origin);

    /**
     * 2021.12.18
     * 绑定线路和压栈
     */
    void actDulplicateTrain(std::shared_ptr<Train> origin);

    void actDulplicatePage(std::shared_ptr<DiagramPage> origin);

};



namespace qecmd {

    class AddRailway :public QUndoCommand {
        DiagramNaviModel* const navi;
        std::shared_ptr<Railway> rail;
    public:
        AddRailway(DiagramNaviModel* navi_, std::shared_ptr<Railway> rail_,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * 添加列车。并不限制是空白的。
     * 构造本类之前，应当先完成绑定操作。
     */
    class AddNewTrain :public QUndoCommand {
        DiagramNaviModel* const navi;
        std::shared_ptr<Train> train;
    public:
        AddNewTrain(DiagramNaviModel* navi_,std::shared_ptr<Train> train_,
            QUndoCommand* parent=nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class BatchAddTrain : public QUndoCommand {
        DiagramNaviModel* const navi;
        QVector<std::shared_ptr<Train>> trains;
    public:
        BatchAddTrain(DiagramNaviModel* navi_, const QVector<std::shared_ptr<Train>>& trains_,
            QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("批量添加%1个车次").arg(trains_.size()),parent),
            navi(navi_),trains(trains_){}

        virtual void undo()override;
        virtual void redo()override;
    };
    

}

#endif
