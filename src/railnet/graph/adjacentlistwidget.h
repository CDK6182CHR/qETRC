#pragma once
#include <QStandardItemModel>
#include <QWidget>
#include "railnet.h"

class ForbidNodesModel;
class RulerNodesModel;
class AdjacentListModel;

class QTableView;
/**
 * @brief The AdjacentListWidget class
 * 查看邻接表的页面，分为正邻接表和反邻接表
 */
class AdjacentListWidget : public QWidget
{
    Q_OBJECT
    AdjacentListModel* const mdIn;
    AdjacentListModel * const mdOut;
    std::shared_ptr<RailNet::vertex> ve;
    QTableView* tbIn,*tbOut,*tbRuler,*tbForbid;
    RulerNodesModel*const mdRuler;
    ForbidNodesModel* const mdForbid;
public:
    explicit AdjacentListWidget(const RailNet& net, QWidget *parent = nullptr);
private:
    void initUI();
public slots:
    void setVertex(std::shared_ptr<RailNet::vertex> ve);

signals:
    void currentEdgeChanged(std::shared_ptr<RailNet::edge> ed);

private slots:
    void onInRowChanged(const QModelIndex& idx);
    void onOutRowChanged(const QModelIndex& idx);
    void onCurrentEdgeChanged(std::shared_ptr<RailNet::edge> ed);
};

