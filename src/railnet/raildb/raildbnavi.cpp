#include "raildbnavi.h"
#include "raildbmodel.h"
#include "data/common/qesystem.h"
#include "raildb.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QUndoStack>
#include <QFileDialog>

#define _RAILDB_NOT_IMPLEMENTED do{  \
QMessageBox::information(this,tr("线路数据库"), tr("此功能尚未实现！"));\
}while(false)

RailDBNavi::RailDBNavi(std::shared_ptr<RailDB> raildb, QWidget *parent):
    QWidget(parent), _raildb(raildb), model(new RailDBModel(raildb, this))
{
    initUI();
    initContext();
    _undo=new QUndoStack(this);
    connect(_undo,&QUndoStack::indexChanged,this,
            &RailDBNavi::markChanged);
    //openDB(SystemJson::instance.default_raildb_file);
}

void RailDBNavi::initUI()
{
    auto* vlay=new QVBoxLayout(this);

    tree=new QTreeView;
    tree->setModel(model);
    connect(tree->selectionModel(),&QItemSelectionModel::currentChanged,
            this,&RailDBNavi::onCurrentChanged);
    vlay->addWidget(tree);
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tree, &QTreeView::customContextMenuRequested,
        this, &RailDBNavi::showContextMenu);

}

void RailDBNavi::initContext()
{
    QAction* act;
    meCat=new QMenu;
    act=meCat->addAction(tr("新建线路"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actNewRail);
    act=meCat->addAction(tr("新建子分类"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actNewSubcat);
    act=meCat->addAction(tr("新建平级分类"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actNewParallelCat);

    meRail=new QMenu;
    act=meRail->addAction(tr("打开基线"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actEditRail);
    act=meRail->addAction(tr("删除"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actRemoveRail);
    act=meRail->addAction(tr("编辑标尺"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actRuler);
    act=meRail->addAction(tr("编辑天窗"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actForbid);
    meRail->addSeparator();
    act=meRail->addAction(tr("导出到运行图"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actExportToDiagram);
}

std::shared_ptr<Railway> RailDBNavi::currentRailway()
{
    auto&& idx=tree->currentIndex();
    auto* it=model->getItem(idx);
    if(it->type()==navi::RailwayItemDB::Type){
        return static_cast<navi::RailwayItemDB*>(it)->railway();
    }
    return nullptr;
}

std::shared_ptr<RailCategory> RailDBNavi::currentCategory()
{
    auto&& idx=tree->currentIndex();
    auto* it=model->getItem(idx);
    if(it->type()==navi::RailCategoryItem::Type){
        return static_cast<navi::RailCategoryItem*>(it)->category();
    }
    return nullptr;
}

void RailDBNavi::showContextMenu(const QPoint &pos)
{
    auto&& idx=tree->currentIndex();
    if(!idx.isValid())return;
    auto* item=model->getItem(idx);
    switch (item->type()) {
    case navi::RailCategoryItem::Type: meCat->popup(tree->mapToGlobal(pos));break;
    case navi::RailwayItemDB::Type: meRail->popup(tree->mapToGlobal(pos));break;
    }
}

void RailDBNavi::actNewRail()
{
    _RAILDB_NOT_IMPLEMENTED;
}

void RailDBNavi::actNewSubcat()
{
    _RAILDB_NOT_IMPLEMENTED;
}

void RailDBNavi::actNewParallelCat()
{
    _RAILDB_NOT_IMPLEMENTED;
}

void RailDBNavi::actEditRail()
{
    if (auto p=currentRailway()){
        emit focusInRailway(p);
    }
}

void RailDBNavi::actRemoveRail()
{
    _RAILDB_NOT_IMPLEMENTED;
}

void RailDBNavi::actRuler()
{
    _RAILDB_NOT_IMPLEMENTED;
}

void RailDBNavi::actForbid()
{
    _RAILDB_NOT_IMPLEMENTED;
}

void RailDBNavi::actExportToDiagram()
{
    auto rail=currentRailway();
    if(rail){
        emit exportRailwayToDiagram(rail);
    }
}

void RailDBNavi::refreshData()
{
    model->resetModel();
}

void RailDBNavi::onCurrentChanged(const QModelIndex &cur, const QModelIndex &prev)
{
    Q_UNUSED(prev)
    auto* it=model->getItem(cur);
    switch (it->type()){
    case navi::RailwayItemDB::Type:
        emit focusInRailway(static_cast<navi::RailwayItemDB*>(it)->railway());break;
    }
}

bool RailDBNavi::openDB(const QString &filename)
{
    clearDBUnchecked();

    RailDB db;
    bool flag=db.parseJson(filename);
    if(flag){
        _raildb->operator=(std::move(db));   // move assign
        afterResetDB();
        return true;
    }else{
        qDebug() << "RaiDBWindow::openDB: WARNING: open failed: " << filename;
        return false;
    }
}

void RailDBNavi::clearDBUnchecked()
{
    _undo->clear();
    _raildb->clear();
}

void RailDBNavi::afterResetDB()
{
    refreshData();
    emit dbReset();
}

bool RailDBNavi::saveQuestion()
{
    auto flag = QMessageBox::question(this, tr("线路数据库"),
        tr("是否保存对线路数据库文件[%1]的更改？").arg(_raildb->filename()),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
    if (flag == QMessageBox::Yes) {
        actSave();
        return true;
    }
    else if (flag == QMessageBox::No) {
        return true;
    }
    else {
        //包括Cancel和NoButton
        return false;
    }

}

void RailDBNavi::markChanged()
{
    if(!_changed){
        _changed=true;
        emit changedFlagChanged(_changed);
    }
}

void RailDBNavi::markUnchanged()
{
    if(_changed){
        _changed=false;
        emit changedFlagChanged(_changed);
    }
}


void RailDBNavi::actOpen()
{
    if (_changed && !saveQuestion())
        return;
    auto f = QFileDialog::getOpenFileName(this, tr("打开线路数据库"), {},
        tr("pyETRC线路数据库文件 (*.pyetlib)\nJSON文件 (*.json)\n"
            "所有文件 (*)"));
    if (f.isEmpty())return;
    bool flag = openDB(f);
    if (!flag) {
        QMessageBox::warning(this, tr("线路数据库"), tr("打开数据库文件失败，或文件为空。"));
    }
}

void RailDBNavi::actNewDB()
{
    if (_changed && !saveQuestion())
        return;
    clearDBUnchecked();
    _raildb->setName("");
    afterResetDB();
}

void RailDBNavi::actSave()
{
    if (_raildb->filename().isEmpty()) {
        actSaveAs();
    }
    else {
        bool flag=_raildb->save();
        if(flag){
            markUnchanged();
            _undo->setClean();
        }
    }
}

void RailDBNavi::actSaveAs()
{
    QString f=QFileDialog::getSaveFileName(this,tr("保存线路数据库"),
         {},tr("pyETRC线路数据库文件 (*.pyetlib)\nJSON文件 (*.json)\n"
            "所有文件 (*)"));
    if(f.isEmpty())
        return;
    bool flag=_raildb->saveAs(f);
    if(flag){
        markUnchanged();
        _undo->setClean();
    }
}

