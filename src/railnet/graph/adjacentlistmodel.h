#pragma once

#include <QStandardItemModel>
#include "railnet.h"

class AdjacentListModel: public QStandardItemModel
{
    Q_OBJECT
    const RailNet& net;
    std::shared_ptr<const RailNet::vertex> ve{};
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
    void setupForInAdj(std::shared_ptr<const RailNet::vertex> v);
    void setupForOutAdj(std::shared_ptr<const RailNet::vertex> v);
private:
    void setupRow(std::shared_ptr<RailNet::edge> e, int row);
};
