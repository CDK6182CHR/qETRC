﻿#include "pathlistwidget.h"
#include "model/trainpath/pathlistmodel.h"
#include "data/trainpath/trainpathcollection.h"


#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QUndoStack>
#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"

PathListWidget::PathListWidget(RailCategory& railcat, TrainPathCollection& pathcoll, QUndoStack* undo, QWidget* parent):
    QWidget(parent), railcat(railcat), pathcoll(pathcoll), _model(new PathListModel(pathcoll,this)), _undo(undo)
{
    initUI();
}

void PathListWidget::updatePath(TrainPath* path)
{
    _model->updatePath(path);
}

void PathListWidget::initUI()
{
    auto* vlay = new QVBoxLayout(this);
    table = new QTableView;
    vlay->addWidget(table);

    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(_model);
    {
        int c = 0;
        for (int w : { 100,40,100,100 }) {
            table->setColumnWidth(c++, w);
        }
    }
    connect(table, &QTableView::doubleClicked,
        this, &PathListWidget::editPathByModelIndex);
    connect(table->selectionModel(), &QItemSelectionModel::currentRowChanged,
        this, &PathListWidget::onCurrentChanged);

    auto* btn = new QPushButton(tr("刷新所有径路可用性"));
    connect(btn, &QPushButton::clicked, this, &PathListWidget::actUpdateAllValidity);
    vlay->addWidget(btn);

    auto* g = new ButtonGroup<3>({ "编辑","添加","删除" });
    g->connectAll(SIGNAL(clicked()), this,
        { SLOT(actEdit()), SLOT(actAdd()), SLOT(actDelete()) });
    vlay->addLayout(g);
}

void PathListWidget::actEdit()
{
    const auto& idx = table->currentIndex();
    editPathByModelIndex(idx);
}

void PathListWidget::actAdd()
{
    auto path = std::make_unique<TrainPath>(pathcoll.validNewPathName(tr("新径路")));
    if (_undo) {
        _undo->push(new qecmd::AddTrainPath(std::move(path), this));
    }
    else {
        commitAddPath(std::move(path));
    }
}

void PathListWidget::actDelete()
{
    const auto& idx = table->currentIndex();
    int row = idx.row();
    if (0 <= row && row < pathcoll.size()) {
        //actRemovePath(row);
        emit removePath(row);
    }
}

void PathListWidget::editPathByModelIndex(const QModelIndex& idx)
{
    if (idx.isValid()) {
        emit editPath(idx.row());
    }
}

void PathListWidget::onCurrentChanged(const QModelIndex& idx)
{
    emit focusInPath(pathcoll.at(idx.row()));
}

void PathListWidget::actUpdateAllValidity()
{
    pathcoll.checkValidAll(railcat);
    _model->updateAllValidity();
}

void PathListWidget::actDuplicate(int idx)
{
    const auto& path = pathcoll.paths().at(idx);
    auto npath = std::make_unique<TrainPath>(*path);   // copy construct!
    npath->setName(pathcoll.validNewPathName(path->name() + QObject::tr("_副本")));
    if (_undo) {
        _undo->push(new qecmd::AddTrainPath(std::move(npath), this));
    }
    else {
        commitAddPath(std::move(npath));
    }
}

void PathListWidget::refreshData()
{
    _model->refreshData();
}

void PathListWidget::commitAddPath(std::unique_ptr<TrainPath>&& path)
{
    auto* p = path.get();
    _model->addPathAt(pathcoll.size(), std::move(path));
    emit emptyPathAdded(p);
}

std::unique_ptr<TrainPath> PathListWidget::undoAddPath()
{
    auto p = _model->popPathAt(pathcoll.size() - 1);
    emit pathRemoved(p.get());
    return p;
}

std::unique_ptr<TrainPath> PathListWidget::commitRemovePath(int idx)
{
    auto p = _model->popPathAt(idx);
    emit pathRemoved(p.get());
    return p;
}

void PathListWidget::undoRemovePath(int idx, std::unique_ptr<TrainPath>&& path)
{
    _model->addPathAt(idx, std::move(path));
}

qecmd::AddTrainPath::AddTrainPath(std::unique_ptr<TrainPath>&& path_, PathListWidget* const pw, QUndoCommand* parent):
    QUndoCommand(QObject::tr("添加径路: %1").arg(path_->name()), parent), 
    path(std::move(path_)),
    pw(pw)
{
}

void qecmd::AddTrainPath::undo()
{
    path = pw->undoAddPath();
}

void qecmd::AddTrainPath::redo()
{
    pw->commitAddPath(std::move(path));
}

