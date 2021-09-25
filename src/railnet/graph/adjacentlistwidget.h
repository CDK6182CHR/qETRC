#pragma once
#include <QStandardItemModel>
#include <QWidget>
#include "railnet.h"

class ForbidNodesModel;
class RulerNodesModel;
class AdjacentListModel: public QStandardItemModel
{
    Q_OBJECT
    const  RailNet& net;
    std::shared_ptr<RailNet::vertex> ve{};
public:
    enum{
        ColRailName=0,
        ColFrom,
        ColTo,
        ColDir,
        ColMile,
        ColRulerCount,
        ColMAX
    };

    AdjacentListModel(const RailNet& net, QObject* parent=nullptr);
    std::shared_ptr<RailNet::edge> edgeFromRow(int row);
public slots:
    void setupForInAdj(std::shared_ptr<RailNet::vertex> v);
    void setupForOutAdj(std::shared_ptr<RailNet::vertex> v);
private:
    void setupRow(std::shared_ptr<RailNet::edge> e, int row);
};

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

