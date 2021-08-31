#pragma once

#include <QWidget>
#include <QMenu>
#include <QPoint>
#include <QUndoStack>
#include <QUndoCommand>
#include <QVector>

#include "data/diagram/diagrampage.h"
#include "model/diagram/diagramnavimodel.h"

class QTreeView;

/**
 * @brief The NaviTree class  系统导航那个TreeView
 * 注意这个类暂定只有主程序才用 （i.e. 不给数据库预留），因此持有主程序的undoStack指针
 */
class NaviTree : public QWidget
{
    Q_OBJECT;

    QMenu* mePageList, * meRailList, * meTrainList, * mePage, * meTrain, * meRailway;
    QMenu* meRuler, * meForbid, * meRouting, * meRoutingList;
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
    void editTrain(std::shared_ptr<Train>);
    void editRuler(std::shared_ptr<Ruler>);
    void editForbid(std::shared_ptr<Forbid>, std::shared_ptr<Railway>);
    void editRouting(std::shared_ptr<Routing>);
    void focusOutRailway();
    void railwayListChanged();   //只是通知主窗口更新数据  其实都不见得有必要

    /**
     * 转发给主窗口去画图  所有页面重新铺画
     */
    void trainsImported();
    void removeRulerNavi(std::shared_ptr<Ruler>);
    void removeRoutingNavi(int row);

    void actAddRouting();
    void actBatchParseRouting();
    void actBatchDetectRouting();

private slots:
    void showContextMenu(const QPoint& pos);
    
    void onDoubleClicked(const QModelIndex& index);

    /**
     * AddPageDialog返回后进入
     */
    void addNewPageApply(std::shared_ptr<DiagramPage> page);

    void onRemovePageContext();

    void onRemoveSingleTrainContext();

    void onRemoveRailwayContext();

    void onRemoveRulerContext();

    void onRemoveRoutingContext();

    void onEditRailwayContext();

    void onEditTrainContext();

    void onEditRulerContext();

    void onEditForbidContext();

    void onEditRoutingContext();

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

    void importRailways();
    void addNewPage();
    
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
    
};



namespace qecmd {
    class ImportRailways:public QUndoCommand {
        DiagramNaviModel* const navi;
        QList<std::shared_ptr<Railway>> rails;
    public:
        ImportRailways(DiagramNaviModel* navi_, const QList<std::shared_ptr<Railway>>& rails_,
            QUndoCommand* parent=nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class AddRailway :public QUndoCommand {
        DiagramNaviModel* const navi;
        std::shared_ptr<Railway> rail;
    public:
        AddRailway(DiagramNaviModel* navi_, std::shared_ptr<Railway> rail_,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class AddNewTrain :public QUndoCommand {
        DiagramNaviModel* const navi;
        std::shared_ptr<Train> train;
    public:
        AddNewTrain(DiagramNaviModel* navi_,std::shared_ptr<Train> train_,
            QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("新建车次: ")+train_->trainName().full(),parent),
            navi(navi_),train(train_){}
        virtual void undo()override {
            navi->undoAddTrain();
        }
        virtual void redo()override {
            navi->commitAddTrain(train);
        }
    };

    class BatchAddTrain : public QUndoCommand {
        DiagramNaviModel* const navi;
        QVector<std::shared_ptr<Train>> trains;
    public:
        BatchAddTrain(DiagramNaviModel* navi_, const QVector<std::shared_ptr<Train>>& trains_,
            QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("批量添加%1个车次").arg(trains_.size()),parent),
            navi(navi_),trains(trains_){}

        virtual void undo()override {
            navi->undoBatchAddTrains(trains.size());
        }
        virtual void redo()override {
            navi->commitBatchAddTrains(trains);
        }
    };

    class RemoveSingleTrain :public QUndoCommand {
        DiagramNaviModel* const navi;
        std::shared_ptr<Train> train;
        int index;
    public:
        RemoveSingleTrain(DiagramNaviModel* navi_,std::shared_ptr<Train> train_, int index_,
            QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("删除列车: ")+train_->trainName().full(),parent),
            navi(navi_),train(train_),index(index_){}
        virtual void undo()override {
            navi->undoRemoveSingleTrain(index, train);
        }
        virtual void redo()override {
            navi->commitRemoveSingleTrain(index);
        }
    };

    

}