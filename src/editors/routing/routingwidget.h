#pragma once

#include <QWidget>
#include <QUndoCommand>

#include "model/train/routingcollectionmodel.h"

class Routing;
class QTableView;
class QUndoStack;

/**
 * @brief The RoutingWidget class
 * pyETRC风格的交路管理面板，全局保持存在。
 */
class RoutingWidget : public QWidget
{
    Q_OBJECT;
    TrainCollection& coll;
    QUndoStack* const _undo;
    RoutingCollectionModel*const model;

    QTableView* table;
public:
    explicit RoutingWidget(TrainCollection& coll_,QUndoStack* undo, QWidget *parent = nullptr);
    void refreshData();
    auto* getModel() { return model; }
private:
    void initUI();

signals:
    void focusInRouting(std::shared_ptr<Routing>);
    void editRouting(std::shared_ptr<Routing>);

    /**
     * 非空的交路添加：进行绘图操作。
     */
    void nonEmptyRoutingAdded(std::shared_ptr<Routing>);

    /**
     * 交路删除：通告进行绘图，以及focusOut，关闭窗口等操作
     */
    void routingRemoved(std::shared_ptr<Routing>);
private slots:
    void actEdit();
    void actAdd();
    void actRemove();

    void actDoubleClicked(const QModelIndex& idx);

    void onCurrentRowChanged(const QModelIndex& idx);

public slots:

    /**
     * 实施添加交路的操作，保证添加到最后一个。
     * 具体操作，交给model。这里同时发射信号，通告改动。
     * 
     */
    void commitAddRouting(std::shared_ptr<Routing> routing);

    /**
     * 撤销添加交路：删除最后一个交路，然后通告改动
     */
    void undoAddRouting(std::shared_ptr<Routing> routing);

    void commitRemoveRouting(int row, std::shared_ptr<Routing> routing);

    void undoRemoveRouting(int row, std::shared_ptr<Routing> routing);

};

namespace qecmd {

    /**
     * 添加交路：要求能处理空的和非空的。
     * 实际的操作由RoutingWidget去完成；然后用SIGNAL通告重新铺画等操作。
     */
    class AddRouting :public QUndoCommand {
        std::shared_ptr<Routing> routing;
        RoutingWidget*const rw;
    public:
        AddRouting(std::shared_ptr<Routing> routing_, RoutingWidget* const rw,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class RemoveRouting :public QUndoCommand {
        int row;
        std::shared_ptr<Routing> routing;
        RoutingWidget* const rw;
    public:
        RemoveRouting(int row_, std::shared_ptr<Routing> routing_, RoutingWidget* const rw_,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };
}