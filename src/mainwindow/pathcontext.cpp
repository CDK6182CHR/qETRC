#include "pathcontext.h"

#ifndef QETRC_MOBILE_2

#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <SARibbonContextCategory.h>
#include <SARibbonLineEdit.h>
#include <SARibbonMenu.h>
#include <DockWidget.h>
#include <DockManager.h>

#include "mainwindow.h"
#include "traincontext.h"
#include "editors/trainpath/pathedit.h"
#include "editors/trainpath/pathlistwidget.h"
#include "data/train/train.h"


PathContext::PathContext(Diagram &diagram, SARibbonContextCategory *cont,
                           MainWindow *mw, QObject *parent):
    QObject(parent), diagram(diagram), cont(cont), mw(mw)
{
    initUI();
}

void PathContext::setPath(TrainPath *path)
{
    this->path=path;
    refreshData();
}

void PathContext::refreshData()
{
    if (!path) {
        qWarning() << "empty path";
        return;
    }

    edName->setText(path->name());
}

void PathContext::initUI()
{
    auto* page=cont->addCategoryPage(tr("列车径路(&A)"));
    auto* panel=page->addPannel(tr(""));

    if constexpr (true) {
        auto* ed = new SARibbonLineEdit;
        edName = ed;

        QWidget* w = new QWidget;
        auto* vlay = new QVBoxLayout;

        auto* lab = new QLabel(tr("当前列车径路"));
        lab->setAlignment(Qt::AlignCenter);
        vlay->addWidget(lab);

        ed->setReadOnly(true);
        ed->setAlignment(Qt::AlignCenter);
        ed->setFixedWidth(100);

        vlay->addWidget(ed);
        w->setLayout(vlay);
        panel->addWidget(w, SARibbonPannelItem::Large);
    }

    panel=page->addPannel(tr("编辑"));

    auto* act=new QAction(QIcon(":/icons/edit.png"), tr("编辑"));
    connect(act,&QAction::triggered, this, &PathContext::actEditPath);
    panel->addLargeAction(act);

    panel = page->addPannel(tr("列车"));
    act = new QAction(qApp->style()->standardIcon(QStyle::SP_TabCloseButton), tr("清空"));
    connect(act, &QAction::triggered, this, &PathContext::actClearTrains);
    panel->addLargeAction(act);

    panel=page->addPannel(tr(""));
    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_TrashIcon),
        tr("删除径路"), this);
    act->setToolTip(tr("删除径路\n删除当前列车径路"));
    panel->addLargeAction(act);
    connect(act, &QAction::triggered, this, &PathContext::actRemovePath);

    act = new QAction(QIcon(":/icons/copy.png"), tr("副本"), this);
    act->setToolTip(tr("创建径路副本\n创建当前列车径路副本"));
    panel->addLargeAction(act);
    connect(act, &QAction::triggered, this, &PathContext::actDuplicatePath);

    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton),
        tr("关闭面板"), this);
    connect(act, &QAction::triggered, mw, &MainWindow::focusOutPath);
    act->setToolTip(tr("关闭面板\n关闭当前的列车径路上下文工具栏页面。"));
    panel->addLargeAction(act);

}

int PathContext::pathEditIndex(const TrainPath* path)
{
    for (int i = 0; i < pathEdits.size(); i++) {
        if (pathEdits.at(i)->path() == path)
            return i;
    }
    return -1;
}

void PathContext::actRemovePath()
{
    int idx = diagram.pathCollection().pathIndex(path);
    removePath(idx);
    //mw->pathListWidget->actRemovePath(idx);
}

void PathContext::actDuplicatePath()
{
    int idx = diagram.pathCollection().pathIndex(path);
    mw->pathListWidget->actDuplicate(idx);
}

void PathContext::actEditPath()
{
    openPathEdit(path);
}


void PathContext::openPathEdit(TrainPath* path)
{
    int i = pathEditIndex(path);
    if (i == -1) {
        //创建
        auto* pw = new PathEdit(diagram.railCategory(), diagram.pathCollection());
        pw->setPath(path);
        auto* dock = new ads::CDockWidget(tr("列车径路 - %1").arg(path->name()));
        dock->setWidget(pw);
        pathEdits.append(pw);
        pathDocks.append(dock);

        connect(pw, &PathEdit::pathApplied,
            this, &PathContext::actUpdatePath);
        connect(mw->pathListWidget, &PathListWidget::pathRemoved,
            this, &PathContext::afterPathRemoved);

        mw->pathMenu->addAction(dock->toggleViewAction());
        mw->getManager()->addDockWidgetFloating(dock);
    }
    else {
        //显示
        auto* dock = pathDocks.at(i);
        if (dock->isClosed()) {
            dock->toggleView(true);
        }
        else {
            dock->setAsCurrentTab();
        }
    }
}

void PathContext::openPathEditIndex(int idx)
{
    auto* path = diagram.pathCollection().at(idx);
    openPathEdit(path);
}

void PathContext::removePathDocks()
{
    foreach(auto p, pathDocks) {
        p->deleteDockWidget();
    }
    pathDocks.clear();
    pathEdits.clear();
}

void PathContext::actUpdatePath(TrainPath* path, std::unique_ptr<TrainPath>& data)
{
    mw->getUndoStack()->push(new qecmd::UpdatePath(path, std::move(data), this));
}

void PathContext::commitUpdatePath(TrainPath* path)
{
    // inform the PathListWidget to update path
    mw->pathListWidget->updatePath(path);

    // update the path editor, if available
    if (int idx = pathEditIndex(path); idx >= 0) {
        auto* ed = pathEdits.at(idx);
        ed->refreshData();
        pathDocks.at(idx)->setWindowTitle(tr("列车径路 - %1").arg(path->name()));
    }
}

void PathContext::afterPathRemoved(TrainPath* path)
{
    int idx = pathEditIndex(path);
    if (idx >= 0) {
        auto* dock = pathDocks.takeAt(idx);
        dock->deleteDockWidget();

        pathEdits.removeAt(idx);
    }
    if (path == this->path) {
        mw->focusOutPath();
    }
}

void PathContext::actClearTrains()
{
    mw->getUndoStack()->push(new qecmd::ClearTrainsFromPath(path, this));
}

void PathContext::afterPathTrainsChanged(TrainPath* path, const std::vector<std::weak_ptr<Train>>& trains)
{
    // rebind and repaint trains
    auto* c = mw->getTrainContext();
    for (auto t : trains) {
        auto train = t.lock();
        c->afterTimetableChanged(train);
    }
    c->refreshPath();

    // update path data (nothing to do for now)
}

void PathContext::removePath(int idx)
{
    auto p = diagram.pathCollection().at(idx);
    mw->getUndoStack()->push(new qecmd::RemoveTrainPath(p, idx, mw->pathListWidget, this));
}



qecmd::UpdatePath::UpdatePath(TrainPath* path, std::unique_ptr<TrainPath>&& data_,
    PathContext* cont, QUndoCommand* parent) :
    QUndoCommand(QObject::tr("更新列车径路: %1").arg(path->name()), parent),
    path(path), data(std::move(data_)), cont(cont)
{
}

void qecmd::UpdatePath::undo()
{
    path->swapDataWith(*data);
    cont->commitUpdatePath(path);
}

void qecmd::UpdatePath::redo()
{
    path->swapDataWith(*data);
    cont->commitUpdatePath(path);
}

qecmd::ClearTrainsFromPath::ClearTrainsFromPath(TrainPath* path, PathContext* cont, QUndoCommand* parent):
    QUndoCommand(QObject::tr("清空径路的列车: %1").arg(path->name()), parent), path(path), cont(cont)
{
    for (auto t : path->trains()) {
        auto train = t.lock();
        int idx = train->getPathIndex(path);
        indexes_in_train.emplace_back(idx);
    }
}

void qecmd::ClearTrainsFromPath::undo()
{
    std::swap(trains, path->trains());
    for (int i = 0; i < path->trains().size(); i++) {
        auto train = path->trains().at(i).lock();
        int idx = indexes_in_train.at(i);
        train->paths().insert(train->paths().begin() + idx, path);
    }
    cont->afterPathTrainsChanged(path, trains);
}

void qecmd::ClearTrainsFromPath::redo()
{
    std::swap(trains, path->trains());
    for (int i = 0; i < trains.size(); i++) {
        auto train = trains.at(i).lock();
        int idx = indexes_in_train.at(i);
        train->paths().erase(train->paths().begin() + idx);
    }
    cont->afterPathTrainsChanged(path, trains);
}


qecmd::RemoveTrainPath::RemoveTrainPath(TrainPath* p, int idx, PathListWidget* pw, PathContext* cont, QUndoCommand* parent) :
    QUndoCommand(QObject::tr("删除径路: %1").arg(p->name()), parent), idx(idx), pw(pw), path(), cont(cont)
{
    if (!p->trains().empty()) {
        new ClearTrainsFromPath(p, cont, this);
    }
}

void qecmd::RemoveTrainPath::undo()
{
    pw->undoRemovePath(idx, std::move(path));
    QUndoCommand::undo();    // re-add the trains
}

void qecmd::RemoveTrainPath::redo()
{
    QUndoCommand::redo();    // clear the trains from path
    path = pw->commitRemovePath(idx);
    cont->afterPathRemoved(path.get());
}


#endif

