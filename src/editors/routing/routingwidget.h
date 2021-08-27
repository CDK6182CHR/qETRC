#pragma once

#include <QWidget>

#include "model/train/routingcollectionmodel.h"

class Routing;
class QTableView;

/**
 * @brief The RoutingWidget class
 * pyETRC风格的交路管理面板，全局保持存在。
 */
class RoutingWidget : public QWidget
{
    Q_OBJECT;
    TrainCollection& coll;
    RoutingCollectionModel*const model;

    QTableView* table;
public:
    explicit RoutingWidget(TrainCollection& coll_, QWidget *parent = nullptr);
    void refreshData();
    auto* getModel() { return model; }
private:
    void initUI();

signals:
    void focusInRouting(std::shared_ptr<Routing>);
    void editRouting(std::shared_ptr<Routing>);
private slots:
    void actEdit();
    void actAdd();
    void actRemove();

    void actDoubleClicked(const QModelIndex& idx);

    void onCurrentRowChanged(const QModelIndex& idx);

};

