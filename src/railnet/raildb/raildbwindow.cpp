#include "raildbwindow.h"
#include "raildb.h"
#include "raildbnavi.h"
#include "raildbmodel.h"
#include "data/rail/railway.h"
#include "model/rail/railstationmodel.h"
#include <limits>

#include <QSplitter>
#include <QUndoStack>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenuBar>
#include <QTreeView>
#include <QUndoView>

#include <editors/railstationwidget.h>
#include "data/common/qesystem.h"

RailDBWindow::RailDBWindow(QWidget *parent) : QMainWindow(parent),
    _raildb(std::make_shared<RailDB>())
{
    setAttribute(Qt::WA_DeleteOnClose);

    initUI();
    connect(navi, &RailDBNavi::focusInRailway, this,
        &RailDBWindow::onNaviRailChanged);
    connect(navi, &RailDBNavi::changedFlagChanged,
            this, &RailDBWindow::updateWindowTitle);
    connect(navi, &RailDBNavi::dbReset,
            this, &RailDBWindow::afterResetDB);

    updateWindowTitle(false);
}

void RailDBWindow::initUI()
{
    resize(1200,800);
    auto* sp=new QSplitter;

    navi=new RailDBNavi(_raildb);
    sp->addWidget(navi);
    connect(navi,&RailDBNavi::exportRailwayToDiagram,
            this,&RailDBWindow::exportRailwayToDiagram);

    undoView = new QUndoView(navi->undoStack(), this);
    undoView->setWindowFlags(Qt::Dialog);
    undoView->setWindowTitle(tr("线路数据库 - 历史记录"));

    editor=new RailStationWidget(*_raildb,false);
    sp->addWidget(editor);
    setCentralWidget(sp);
    // 平均分配
    sp->setSizes(QList<int>{ std::numeric_limits<int>::max(),std::numeric_limits<int>::max() });

    connect(editor, &RailStationWidget::railNameChanged,
        this, &RailDBWindow::onEditorRailNameChanged);
    connect(editor->getModel(), &RailStationModel::actStationTableChanged,
        this, &RailDBWindow::onEditorStationChanged);
    connect(editor, &RailStationWidget::invalidApplyRequest,
        this, &RailDBWindow::onEditorInvalidApplied);

    initMenuBar();
}

void RailDBWindow::initMenuBar()
{
    auto* menubar = new QMenuBar(this);
    setMenuBar(menubar);
    QAction* act;

    auto* menu = menubar->addMenu(tr("文件"));
    menu->addAction(tr("新建"), navi, &RailDBNavi::actNewDB);
    menu->addAction(tr("打开"), navi, &RailDBNavi::actOpen);
    menu->addAction(tr("保存"), navi, &RailDBNavi::actSave);
    menu->addAction(tr("另存为"), navi, &RailDBNavi::actSaveAs);
    menu->addSeparator();
    menu->addAction(tr("将当前数据库文件设为默认文件"), navi, &RailDBNavi::actSetAsDefaultFile);
    menu->addSeparator();

    menu = menubar->addMenu(tr("编辑"));
    menu->addAction(navi->undoStack()->createUndoAction(menu, tr("撤销")));
    menu->addAction(navi->undoStack()->createRedoAction(menu, tr("重做")));
    menu->addAction(tr("历史记录"), undoView, &QUndoView::show);
    menu->addSeparator();
    menu->addAction(tr("编辑当前线路"), navi, &RailDBNavi::actEditRail);
    
    menu = menubar->addMenu(tr("查看"));
    menu->addAction(tr("刷新线路表"), navi, &RailDBNavi::refreshData);
    menu->addAction(tr("全部展开"), navi->getTree(), &QTreeView::expandAll);
    menu->addAction(tr("全部折叠"), navi->getTree(), &QTreeView::collapseAll);

}


void RailDBWindow::updateWindowTitle(bool changed)
{
    QString filename = _raildb->filename();
    if (filename.isNull())
        filename = "新线路数据库";
    //暂定用自己维护的标志位来判定修改
    if (changed)
        filename.prepend("* ");
    setWindowTitle(tr("线路数据库 - %1").arg(filename));
}

void RailDBWindow::onNaviRailChanged(std::shared_ptr<Railway> railway,
                                     const std::deque<int> &path)
{
    if (auto p = editor->getRailway();p && editor->isChanged()) {
        auto flag = QMessageBox::question(this, tr("线路数据库"),
            tr("当前编辑的线路[%1]可能已经被修改。是否保存改动？").arg(p->name()),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (flag == QMessageBox::Yes) {
            bool succ = editor->applyChange();  // 注意提交更改可能失败。如果失败，不做任何操作
            if (!succ)return;
        }
        else if (flag == QMessageBox::No) {
            // nothing to do here
        }
        else return;
    }
    editorPath=path;
    editor->setRailway(railway);
}

#define _RAILDB_NOT_IMPLEMENTED do{  \
QMessageBox::information(this,tr("线路数据库"), tr("此功能尚未实现！"));\
}while(false)

void RailDBWindow::onEditorRailNameChanged(std::shared_ptr<Railway> railway, const QString& name)
{
    navi->undoStack()->push(new qecmd::UpdateRailNameDB(railway, name, editorPath, this));
}

void RailDBWindow::onEditorStationChanged(std::shared_ptr<Railway> railway,
    std::shared_ptr<Railway> newtable, bool equiv)
{
    Q_UNUSED(equiv)
    navi->undoStack()->push(new qecmd::UpdateRailStationsDB(railway, newtable,
        editorPath, this));
}

void RailDBWindow::onEditorInvalidApplied()
{
    QMessageBox::warning(this, tr("错误"), tr("当前没有在编辑的线路。请先选择要编辑的线路，"
        "或新建线路，再提交。"));
}

void RailDBWindow::commitUpdateRailName(std::shared_ptr<Railway> railway, const std::deque<int>& path)
{
    if (editor->getRailway() == railway) {
        editor->refreshBasicData();
    }
    navi->getModel()->onRailInfoChanged(railway, path);
}

void RailDBWindow::commitUpdateStations(std::shared_ptr<Railway> railway, const std::deque<int> &path)
{
    if (editor->getRailway() == railway) {
        editor->refreshData();
    }
    navi->getModel()->onRailInfoChanged(railway, path);
}

void RailDBWindow::afterResetDB()
{
    navi->refreshData();
    editorPath = {};   // 构造一个无效的index
    editor->setRailway(nullptr);
    editor->refreshData();
}

qecmd::UpdateRailStationsDB::UpdateRailStationsDB(std::shared_ptr<Railway> railway_,
    std::shared_ptr<Railway> table_, const std::deque<int>& path_, 
    RailDBWindow* wnd_, QUndoCommand* parent):
    QUndoCommand(QObject::tr("更新线路站表: %1").arg(railway_->name()),parent),
    railway(railway_),table(table_),path(path_),wnd(wnd_)
{
}

void qecmd::UpdateRailStationsDB::undo()
{
    railway->swapBaseWith(*table);
    wnd->commitUpdateStations(railway, path);
}

void qecmd::UpdateRailStationsDB::redo()
{
    railway->swapBaseWith(*table);
    wnd->commitUpdateStations(railway, path);
}

qecmd::UpdateRailNameDB::UpdateRailNameDB(std::shared_ptr<Railway> railway_, 
    const QString& name_, const std::deque<int>& path_, RailDBWindow* wnd_, 
    QUndoCommand* parent):
    QUndoCommand(QObject::tr("更改线名: %1").arg(name_),parent),
    railway(railway_),name(name_),path(path_),wnd(wnd_)
{
}

void qecmd::UpdateRailNameDB::undo()
{
    std::swap(railway->nameRef(), name);
    wnd->commitUpdateRailName(railway, path);
}
void qecmd::UpdateRailNameDB::redo()
{
    std::swap(railway->nameRef(), name);
    wnd->commitUpdateRailName(railway, path);
}