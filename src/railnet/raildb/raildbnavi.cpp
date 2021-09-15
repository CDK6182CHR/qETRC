#include "raildbnavi.h"
#include "raildbmodel.h"
#include "data/common/qesystem.h"
#include "raildb.h"
#include "data/rail/railway.h"
#include "data/rail/forbid.h"
#include "editors/forbidwidget.h"

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

    int c = 0;
    for (int w : {200, 80, 120, 120, 120, 120}) {
        tree->setColumnWidth(c++, w);
    }
}

void RailDBNavi::initContext()
{
    QAction* act;
    meCat=new QMenu;
    meCat->addAction(tr("展开"), this, &RailDBNavi::actExpand);
    meCat->addAction(tr("折叠"), this, &RailDBNavi::actCollapse);
    meCat->addSeparator();

    act=meCat->addAction(tr("新建线路"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actNewRail);
    act=meCat->addAction(tr("新建子分类"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actNewSubcat);
    act=meCat->addAction(tr("新建平级分类"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actNewParallelCat);

    meRail=new QMenu;
    act=meRail->addAction(tr("编辑"));
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
    
    meRail->addSeparator();
    meRail->addAction(tr("新建线路"), this, &RailDBNavi::actNewRail);
}

std::shared_ptr<Railway> RailDBNavi::currentRailway()
{
    auto&& idx=tree->currentIndex();
    if (!idx.isValid())return nullptr;
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

RailDBNavi::ACI *RailDBNavi::currentItem()
{
    return model->getItem(tree->currentIndex());
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
    auto* it = currentItem();
    auto idx = tree->currentIndex();
    if (it->type() == navi::RailwayItemDB::Type) {
        it = it->parent();
        idx = idx.parent();
    }
    auto path = it->path();
    path.emplace_back(it->childCount());
    auto railway = std::make_shared<Railway>(_raildb->validRailwayNameRec(tr("新线路")));
    _undo->push(new qecmd::InsertRailDB(railway, path, model));
    tree->setCurrentIndex(model->index(it->childCount() - 1, 0, idx));
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
    if(auto* it=currentItem();it->type()==navi::RailwayItemDB::Type){
        emit focusInRailway(static_cast<navi::RailwayItemDB*>(it)->railway(),
                            it->path());
    }
}

void RailDBNavi::actRemoveRail()
{
    auto idx = tree->currentIndex();
    auto rail = currentRailway();
    if (!rail)return;
    auto* it = static_cast<ACI*>(idx.internalPointer());
    _undo->push(new qecmd::RemoveRailDB(rail, it->path(), model));
}

void RailDBNavi::actRuler()
{
    _RAILDB_NOT_IMPLEMENTED;
}

void RailDBNavi::actForbid()
{
    auto rail = currentRailway();
    if (!rail)return;
    auto* w = new ForbidTabWidget(rail, false, this);
    w->resize(600, 600);
    w->setWindowTitle(tr("天窗编辑 - %1").arg(rail->name()));
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setWindowFlags(Qt::Dialog);
    connect(w, &ForbidTabWidget::forbidChanged,
        this, &RailDBNavi::actChangeForbid);
    w->show();
}

void RailDBNavi::actExportToDiagram()
{
    auto rail=currentRailway();
    if(rail){
        //auto flag = QMessageBox::question(this, tr("线路数据库"),
        //    tr("是否确认将当前选中的线路[%1]完整添加到当前运行图文件？\n"
        //        "如果需要，添加后请手动新建或运行图视窗，以显示运行图。")
        //.arg(rail->name()));
        //if (flag == QMessageBox::Yes) {
        //    emit exportRailwayToDiagram(rail);
        //}
        emit exportRailwayToDiagram(rail);
        
    }
}

void RailDBNavi::actSetAsDefaultFile()
{
    if (_raildb->filename().isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("当前数据库文件名为空（可能是新建的数据库），"
            "不可设置为默认文件名。"));
    }
    else {
        SystemJson::instance.default_raildb_file = _raildb->filename();
        QMessageBox::information(this, tr("提示"), tr("已将当前文件名[%1]设置为默认的"
            "线路数据库文件名。\n"
            "此信息保存在system.json配置文件中。本次关闭主程序后，下次在主程序中使用"
            "线路数据库功能时，将默认打开本文件。").arg(_raildb->filename()));
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
        emit focusInRailway(static_cast<navi::RailwayItemDB*>(it)->railway(),
                            it->path());break;
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

void RailDBNavi::actExpand()
{
    tree->expand(tree->currentIndex());
}

void RailDBNavi::actCollapse()
{
    tree->collapse(tree->currentIndex());
}

void RailDBNavi::actChangeForbid(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> data)
{
    _undo->push(new qecmd::UpdateForbidDB(forbid, data));
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

qecmd::RemoveRailDB::RemoveRailDB(std::shared_ptr<Railway> railway_, 
    const std::deque<int>& path_, RailDBModel* model_, QUndoCommand* parent):
    QUndoCommand(QObject::tr("删除线路: %1").arg(railway_->name()), parent),
    railway(railway_),path(path_),model(model_)
{
}

void qecmd::RemoveRailDB::undo()
{
    model->commitInsertRailwayAt(railway, path);
}

void qecmd::RemoveRailDB::redo()
{
    model->commitRemoveRailwayAt(railway, path);
}

qecmd::InsertRailDB::InsertRailDB(std::shared_ptr<Railway> railway_, 
    const std::deque<int>& path_, RailDBModel* model_, QUndoCommand* parent):
    QUndoCommand(QObject::tr("新建线路: %1").arg(railway_->name()),parent),
    railway(railway_),path(path_),model(model_)
{
}

void qecmd::InsertRailDB::undo()
{
    model->commitRemoveRailwayAt(railway, path);
}

void qecmd::InsertRailDB::redo()
{
    model->commitInsertRailwayAt(railway, path);
}

qecmd::UpdateForbidDB::UpdateForbidDB(std::shared_ptr<Forbid> forbid, 
    std::shared_ptr<Railway> data, QUndoCommand* parent):
    QUndoCommand(QObject::tr("更新天窗: %1 @%2").arg(forbid->name(),forbid->railway().name()),
        parent),forbid(forbid),data(data)
{
}

void qecmd::UpdateForbidDB::undo()
{
    forbid->swap(*data->getForbid(0));
}
void qecmd::UpdateForbidDB::redo()
{
    forbid->swap(*data->getForbid(0));
}
