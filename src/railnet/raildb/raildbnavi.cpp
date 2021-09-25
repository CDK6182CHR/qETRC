#include "raildbnavi.h"
#include "raildbmodel.h"
#include "data/common/qesystem.h"
#include "raildb.h"
#include "data/rail/railway.h"
#include "data/rail/forbid.h"
#include "editors/forbidwidget.h"
#include "editors/ruler/rulerwidget.h"
#include "data/rail/ruler.h"
#include "data/diagram/diagram.h"
#include "util/buttongroup.hpp"

#include <QTreeView>
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QUndoStack>
#include <QFileDialog>
#include <QInputDialog>

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

bool RailDBNavi::deactivate()
{
    auto flag = QMessageBox::question(this, tr("线路数据库"), tr("此操作退出线路数据库，"
        "关闭已打开的数据库文件并释放内存。如果下次再启动线路数据库，将重新读取。"
        "是否继续？"));
    if (flag != QMessageBox::Yes)
        return false;
    else return deactiveOnClose();
}

bool RailDBNavi::deactiveOnClose()
{
    if (_changed && !saveQuestion())
        return false;
    clearDBUnchecked();
    emit dbReset();
    emit deactivated();
    return true;
}

void RailDBNavi::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* hlay=new QHBoxLayout;
    edSearch=new QLineEdit;
    hlay->addWidget(edSearch);
    auto* g=new ButtonGroup<3>({"搜索全站名","搜索部分站名","搜索线名"});
    hlay->addLayout(g);
    vlay->addLayout(hlay);
    g->connectAll(SIGNAL(clicked()),this,
                  {SLOT(searchFullName()),SLOT(searchPartName()),
                  SLOT(searchRailName())});

    tree=new QTreeView;
    tree->setModel(model);
    //tree->setSelectionMode(QTreeView::ContiguousSelection);
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
    meCat->addAction(tr("重命名"), this, &RailDBNavi::actRenameCategory);
    meCat->addSeparator();

    act=meCat->addAction(tr("新建线路"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actNewRail);
    act=meCat->addAction(tr("新建子分类"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actNewSubcat);
    act=meCat->addAction(tr("新建平级分类"));
    connect(act,&QAction::triggered,this,&RailDBNavi::actNewParallelCat);
    meCat->addAction(tr("删除分类"), this, &RailDBNavi::actRemoveCategory);
    meCat->addSeparator();
    meCat->addAction(tr("导出为qETRC多线路运行图文件"), this,
        &RailDBNavi::actExportCategoryToDiagramFile);
    meCat->addAction(tr("导出为子数据库文件"), this,
        &RailDBNavi::actExportCategoryToLib);
    meCat->addSeparator();
    meCat->addAction(tr("从运行图文件导入线路"), this, &RailDBNavi::actImportFromDiagram);
    meCat->addAction(tr("导入子数据库文件"), this, &RailDBNavi::actImportFromLib);

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
    meRail->addAction(tr("导出为独立运行图文件"), this, &RailDBNavi::actExportRailToFile);
    
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
    auto* it = currentItem();
    if (!it)return;
    if (it->type() == navi::RailwayItemDB::Type) it = it->parent();
    insertSubcatOf(it);
}


void RailDBNavi::insertSubcatOf(ACI* it)
{
    if (!it)return;
    auto* cit = static_cast<navi::RailCategoryItem*>(it);
    auto cat = cit->category();
    auto path = cit->path();
    path.emplace_back(cit->subCategoriesCount());

    auto subcat = std::make_shared<RailCategory>(_raildb->validCategoryNameRec(tr("新分类")));
    _undo->push(new qecmd::InsertCategory(subcat, path, model));
}

void RailDBNavi::actNewParallelCat()
{
    auto* it = currentItem();
    if (!it)return;
    insertSubcatOf(it->parent());
}

void RailDBNavi::actRemoveCategory()
{
    auto it = currentItem();
    if (!it) return;
    if (it->type() == navi::RailwayItemDB::Type) it = it->parent();
    const auto& path = it->path();
    auto cat = static_cast<navi::RailCategoryItem*>(it)->category();

    _undo->push(new qecmd::RemoveCategory(cat, path, model));
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
    QStringList items;
    auto railway = currentRailway();
    if (!railway)
        return;
    foreach(auto ruler, railway->rulers()) {
        items.push_back(ruler->name());
    }
    items.push_back(tr("(新建标尺)"));
    bool ok;
    QString res = QInputDialog::getItem(this, tr("编辑标尺"), tr("当前编辑数据库中线路[%1]的标尺。"
        "请选择一个既有标尺，或新建标尺。").arg(railway->name()), items, 0, false, &ok);
    if (!ok)return;
    if (res == tr("(新建标尺)")) {
        // 新建操作
        _undo->push(new qecmd::AddNewRulerDB(railway, 
            railway->validRulerName(tr("新标尺")), this));
    }
    else {
        openRulerWidget(railway->rulerByName(res));
    }
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

void RailDBNavi::actExportRailToFile()
{
    auto rail = currentRailway();
    if (!rail)return;
    QString res = QFileDialog::getSaveFileName(this, tr("导出单线路到运行图"),
        rail->name(), tr("qETRC/pyETRC运行图文件 (*.pyetgr)\nJSON文件 (*.json)\n所有文件 (*)"));
    if (res.isEmpty())return;
    
    Diagram dia;
    dia.addRailway(rail);
    bool flag = dia.saveAs(res);
    if (flag) {
        QMessageBox::information(this, tr("提示"), tr("导出成功"));
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("导出失败"));
    }
}

void RailDBNavi::actExportCategoryToDiagramFile()
{
    auto cat = currentCategory();
    if (!cat)return;
    auto res = QFileDialog::getSaveFileName(this, tr("导出分类到运行图文件"),
        cat->name(), tr("qETRC/pyETRC运行图文件 (*.pyetgr)\nJSON文件 (*.json)\n所有文件 (*)"));
    if (res.isEmpty())return;
    Diagram dia;
    dia.railways().append(cat->railways());
    bool flag = dia.saveAs(res);
    if (flag) {
        QMessageBox::information(this, tr("提示"), tr("导出成功"));
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("导出失败"));
    }
}

void RailDBNavi::actExportCategoryToLib()
{
    auto cat = currentCategory();
    if (!cat)return;
    auto res = QFileDialog::getSaveFileName(this, tr("导出分类为子数据库文件"),
        cat->name(), tr("pyETRC/qETRC线路数据库文件 (*.pyetlib)\nJSON文件 (*.json)\n"
            "所有文件 (*)"));
    if (res.isEmpty())return;
    RailDB subdb = cat->shallowCopy();    // move construct
    bool flag = subdb.saveAs(res);
    if (flag) {
        QMessageBox::information(this, tr("提示"), tr("导出成功"));
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("导出失败"));
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

void RailDBNavi::actImportFromDiagram()
{
    auto* it = currentItem();
    if (!it)return;
    if (it->type() == navi::RailwayItemDB::Type)it = it->parent();
    auto cat = static_cast<navi::RailCategoryItem*>(it)->category();

    auto res = QFileDialog::getOpenFileName(this, tr("从运行图文件导入线路"), {},
        tr("qETRC/pyETRC列车运行图文件 (*.pyetgr)\nJSON文件 (*.json)\n所有文件 (*)"));
    if (res.isEmpty())return;

    Diagram dia;
    bool flag = dia.fromJson(res);
    if (flag && ! dia.railways().isEmpty()) {
        QList<std::shared_ptr<Railway>> lst = std::move(dia.railways());
        foreach(auto p, lst) {
            p->setName(_raildb->validRailwayNameRec(p->name()));
        }
        navi::path_t path = it->path();
        path.emplace_back(it->childCount());
        _undo->push(new qecmd::ImportRailsDB(lst, path, model));
    }

    else {
        QMessageBox::warning(this, tr("错误"), tr("文件格式错误或为空，无法读取线路数据。"));
    }
}

void RailDBNavi::actImportFromLib()
{
    auto it = currentItem();
    if (!it)return;
    if (it->type() == navi::RailwayItemDB::Type) it = it->parent();
    auto* cit = static_cast<navi::RailCategoryItem*>(it);
    auto path = cit->path();
    path.push_back(cit->subCategoriesCount());

    auto filename = QFileDialog::getOpenFileName(this, tr("从线路数据库导入"), {},
        tr("qETRC/pyETRC线路数据库文件 (*.pyetlib)\nJSON文件 (*.json)\n所有文件 (*)"));
    if (filename.isEmpty())return;

    RailDB subdb;
    bool flag = subdb.parseJson(filename);
    if (flag && ! subdb.isNull()) {
        // 直接移动给新的对象了
        auto cat = std::make_shared<RailCategory>(std::move(subdb));
        cat->setName(_raildb->validCategoryNameRec(filename.split('.').front()));
        foreach(auto p, cat->railways()) {
            p->setName(_raildb->validRailwayNameRec(p->name()));
        }
        _undo->push(new qecmd::InsertCategory(cat, path, model));
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("文件错误或为空，无法导入"));
    }
}

void RailDBNavi::actRenameCategory()
{
    auto* it = currentItem();
    if (!it || it->type() != navi::RailCategoryItem::Type)
        return;
    auto* cit = static_cast<navi::RailCategoryItem*>(it);
    auto cat = cit->category();

    bool ok;
    auto name = QInputDialog::getText(this, tr("重命名分类"), tr("将分类[%1]重命名为：")
        .arg(cat->name()), QLineEdit::Normal, cat->name(), &ok);
    if (!ok)return;
    if (name == cat->name()) {
        QMessageBox::information(this, tr("提示"), tr("分类重命名：未做更改。"));
    }
    else if (!_raildb->categoryNameIsValidRec(name, cat)) {
        QMessageBox::warning(this, tr("错误"), tr("名称无效：名称不能为空，且不能与"
            "既有线路、分类名称重复。"));
    }
    else {
        _undo->push(new qecmd::UpdateCategoryNameDB(cat, name, it->path(), model));
    }
}

void RailDBNavi::openRulerWidget(std::shared_ptr<Ruler> ruler)
{
    auto* w = new RulerWidget(ruler, false, this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setWindowFlags(Qt::Dialog);
    w->resize(500, 600);
    w->setWindowTitle(tr("标尺编辑 @ %1").arg(ruler->railway().name()));
    connect(w, &RulerWidget::actChangeRulerName,
        this, &RailDBNavi::actChangeRulerName);
    connect(w, &RulerWidget::actChangeRulerData,
        this, &RailDBNavi::actUpdateRulerData);
    connect(w, &RulerWidget::actRemoveRuler,
        this, &RailDBNavi::actRemoveRuler);
    rulerWidgets.push_back(w);

    w->show();
}

void RailDBNavi::closeRulerWidget(std::shared_ptr<Ruler> ruler)
{
    for (int i = 0; i < rulerWidgets.size();) {
        auto w = rulerWidgets.at(i);
        if (!w || w->getRuler() == ruler) {
            // 同时清除掉已经无效的指针
            rulerWidgets.removeAt(i);
            if (w) {
                w->close();
            }
        }
        else i++;
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
    _changed = false;
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

void RailDBNavi::actChangeRulerName(std::shared_ptr<Ruler> ruler, const QString& name)
{
    _undo->push(new qecmd::ChangeRulerNameDB(ruler, name));
}

void RailDBNavi::actUpdateRulerData(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> data)
{
    _undo->push(new qecmd::UpdateRulerDataDB(ruler, data));
}

void RailDBNavi::actRemoveRuler(std::shared_ptr<Ruler> ruler)
{
    auto data = ruler->clone();
    _undo->push(new qecmd::RemoveRulerDB(ruler, data, ruler->isOrdinateRuler(), this));
}

void RailDBNavi::searchFullName()
{
    searchResult(model->searchFullName(edSearch->text()));
}

void RailDBNavi::searchPartName()
{
    searchResult(model->searchPartName(edSearch->text()));
}

void RailDBNavi::searchRailName()
{
    searchResult(model->searchRailName(edSearch->text()));
}

void RailDBNavi::searchResult(const std::deque<std::deque<int> > &paths)
{
    if(paths.empty()){
        QMessageBox::warning(this,tr("错误"),tr("无符合条件线路"));
    }else if(paths.size()==1){
        tree->setCurrentIndex(model->indexByPath(paths.front()));
    }else{
        QMap<QString, navi::path_t> nameMap;
        QStringList lst;
        for(const auto& p:paths){
            auto rail=model->railwayByPath(p);
            if(!rail){
                qDebug()<<"RailDBNavi::searchResults: WARNING: invalid path"<<
                          Qt::endl;
            }else{
                lst.push_back(rail->name());
                nameMap.insert(rail->name(), p);
            }
        }
        bool ok;
        QString res=QInputDialog::getItem(this,tr("搜索结果"),tr("有多个选项符合条件，请选择："),
                              lst,0,false,&ok);
        if(!ok) return;
        auto path=nameMap.value(res);
        tree->setCurrentIndex(model->indexByPath(path));
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

qecmd::AddNewRulerDB::AddNewRulerDB(std::shared_ptr<Railway> railway,
    const QString& name, RailDBNavi* navi, QUndoCommand* parent):
    QUndoCommand(QObject::tr("新建标尺: %1 @ %2").arg(name,railway->name()),parent),
    railway(railway),name(name),navi(navi)
{
}

void qecmd::AddNewRulerDB::undo()
{
    auto r = railway->takeLastRuler();
    navi->closeRulerWidget(r);
}

void qecmd::AddNewRulerDB::redo()
{
    if (!theRuler) {
        theRuler = railway->addEmptyRuler(name, true);
        theData = theRuler->clone();
    }
    else {
        railway->restoreRulerFrom(theRuler, theData->getRuler(0));
    }
    navi->openRulerWidget(theRuler);
}

qecmd::ChangeRulerNameDB::ChangeRulerNameDB(std::shared_ptr<Ruler> ruler, 
    const QString& name, QUndoCommand* parent):
    QUndoCommand(QObject::tr("更改标尺名称: %1 @ %2").arg(name,ruler->railway().name()),parent),
    ruler(ruler),name(name)
{
}

void qecmd::ChangeRulerNameDB::undo()
{
    std::swap(ruler->nameRef(), name);
}
void qecmd::ChangeRulerNameDB::redo()
{
    std::swap(ruler->nameRef(), name);
}

qecmd::UpdateRulerDataDB::UpdateRulerDataDB(std::shared_ptr<Ruler> ruler, 
    std::shared_ptr<Railway> data, QUndoCommand* parent):
    QUndoCommand(QObject::tr("更新标尺: %1 @ %2").arg(ruler->name(),ruler->railway().name()),
        parent),ruler(ruler),data(data)
{
}

void qecmd::UpdateRulerDataDB::undo()
{
    ruler->swap(*data->getRuler(0));
}
void qecmd::UpdateRulerDataDB::redo()
{
    ruler->swap(*data->getRuler(0));
}

qecmd::RemoveRulerDB::RemoveRulerDB(std::shared_ptr<Ruler> ruler,
    std::shared_ptr<Railway> data, bool isOrd, RailDBNavi* navi, QUndoCommand* parent):
    QUndoCommand(QObject::tr("删除标尺: %1 @ %2").arg(ruler->name(),ruler->railway().name()),
        parent),ruler(ruler),data(data),isOrdinate(isOrd),navi(navi)
{
}

void qecmd::RemoveRulerDB::undo()
{
    ruler->railway().undoRemoveRuler(ruler, data);
    if (isOrdinate) {
        ruler->railway().setOrdinate(ruler);
    }
}

void qecmd::RemoveRulerDB::redo()
{
    ruler->railway().removeRuler(ruler);
    navi->closeRulerWidget(ruler);
}

qecmd::ImportRailsDB::ImportRailsDB(const QList<std::shared_ptr<Railway>>& rails, 
    const std::deque<int>& path, RailDBModel* model, QUndoCommand* parent):
    QUndoCommand(QObject::tr("导入%1条线路").arg(rails.size()),parent),
    rails(rails),path(path),model(model)
{
}

void qecmd::ImportRailsDB::undo()
{
    model->commitRemoveRailwaysAt(rails, path);
}

void qecmd::ImportRailsDB::redo()
{
    model->commitInsertRailwaysAt(rails, path);
}

qecmd::InsertCategory::InsertCategory(std::shared_ptr<RailCategory> cat, 
    const std::deque<int>& path, RailDBModel* model, QUndoCommand* parent):
    QUndoCommand(QObject::tr("插入分类: %1").arg(cat->name()),parent),
    cat(cat),path(path),model(model)
{
}

void qecmd::InsertCategory::undo()
{
    model->commitRemoveCategoryAt(cat, path);
}

void qecmd::InsertCategory::redo()
{
    model->commitInsertCategoryAt(cat, path);
}

qecmd::RemoveCategory::RemoveCategory(std::shared_ptr<RailCategory> cat, 
    const std::deque<int>& path, RailDBModel* model, QUndoCommand* parent):
    QUndoCommand(QObject::tr("删除分类: %1").arg(cat->name()),parent),
    cat(cat),path(path),model(model)
{
}

void qecmd::RemoveCategory::undo()
{
    model->commitInsertCategoryAt(cat, path);
}

void qecmd::RemoveCategory::redo()
{
    model->commitRemoveCategoryAt(cat, path);
}

qecmd::UpdateCategoryNameDB::UpdateCategoryNameDB(std::shared_ptr<RailCategory> cat, 
    const QString& name, const std::deque<int>& path, RailDBModel* model, 
    QUndoCommand* parent):
    QUndoCommand(QObject::tr("更新分类名: %1").arg(cat->name()),parent),
    cat(cat),name(name),path(path),model(model)
{
}

void qecmd::UpdateCategoryNameDB::undo()
{
    std::swap(cat->nameRef(), name);
    model->onCategoryInfoChanged(cat, path);
}
void qecmd::UpdateCategoryNameDB::redo()
{
    std::swap(cat->nameRef(), name);
    model->onCategoryInfoChanged(cat, path);
}
