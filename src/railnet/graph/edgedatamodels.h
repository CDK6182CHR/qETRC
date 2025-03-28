﻿#pragma once
#include <QStandardItemModel>
#include "railnet.h"

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
    Q_OBJECT
public:
    enum{
        ColBegin=0,
        ColEnd,
        ColDuration,
        ColMAX
    };
    ForbidNodesModel(QObject* parent=nullptr);
    void setupModel(std::shared_ptr<RailNet::edge> ed);
};
