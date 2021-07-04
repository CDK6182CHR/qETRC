#pragma once

#include <QTreeView>
#include <QMenu>
#include <QPoint>
#include <QUndoStack>
#include <QUndoCommand>

#include "data/diagram/diagrampage.h"
#include "model/diagram/diagramnavimodel.h"

/**
 * @brief The NaviTree class  系统导航那个TreeView
 * 注意这个类暂定只有主程序才用 （i.e. 不给数据库预留），因此持有主程序的undoStack指针
 */
class NaviTree : public QTreeView
{
    Q_OBJECT;

    QMenu* mePageList, *meRailList;
    DiagramNaviModel* _model;
    QUndoStack* const _undo;
public:
    NaviTree(DiagramNaviModel* model_, QUndoStack* undo, QWidget* parent = nullptr);

protected:
    virtual void currentChanged(const QModelIndex& cur, const QModelIndex& prev)override;

private:
    void initContextMenus();
    using ACI = navi::AbstractComponentItem;
    ACI* getItem(const QModelIndex& idx);


signals:
    void pageAdded(std::shared_ptr<DiagramPage> page);
    void focusInPage(std::shared_ptr<DiagramPage> page);
    void focusOutPage();
    void focusInTrain(std::shared_ptr<Train> train);
    void focusOutTrain();
    void focusInRailway(std::shared_ptr<Railway>);
    void editRailway(std::shared_ptr<Railway>);
    void focusOutRailway();
    void railwayListChanged();   //只是通知主窗口更新数据  其实都不见得有必要

private slots:
    void showContextMenu(const QPoint& pos);
    
    void onDoubleClicked(const QModelIndex& index);

public slots:
    void importRailways();
    void addNewPage();
    
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
}