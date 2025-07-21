#pragma once
#include <QStandardItemModel>
#include "railnet.h"

struct DiagramOptions;

class RulerNodesModel:
        public QStandardItemModel
{
    Q_OBJECT
public:
    enum {
        ColName=0,
        ColPass,
        ColStart,
        ColStop,
        ColSpeed,
        ColMAX
    };
    RulerNodesModel(QObject* parent=nullptr);
    void setupModel(std::shared_ptr<RailNet::edge> ed);
};


class ForbidNodesModel:
        public QStandardItemModel
{
    Q_OBJECT;
    const DiagramOptions& _ops;
public:
    enum{
        ColBegin=0,
        ColEnd,
        ColDuration,
        ColMAX
    };
    ForbidNodesModel(const DiagramOptions& ops, QObject* parent=nullptr);
    void setupModel(std::shared_ptr<RailNet::edge> ed);
};
