#include "raildbwindow.h"
#include "raildb.h"
#include "raildbnavi.h"
#include "data/rail/railway.h"
#include <limits>

#include <QSplitter>
#include <QUndoStack>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenuBar>

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

    initMenuBar();

    navi=new RailDBNavi(_raildb);
    sp->addWidget(navi);
    connect(navi,&RailDBNavi::exportRailwayToDiagram,
            this,&RailDBWindow::exportRailwayToDiagram);

    editor=new RailStationWidget(*_raildb,false);
    sp->addWidget(editor);
    setCentralWidget(sp);
    // 平均分配
    sp->setSizes(QList<int>{ std::numeric_limits<int>::max(),std::numeric_limits<int>::max() });
}

void RailDBWindow::initMenuBar()
{
    auto* menubar = new QMenuBar(this);
    setMenuBar(menubar);

    auto* menu = menubar->addMenu(tr("文件"));
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

void RailDBWindow::onNaviRailChanged(std::shared_ptr<Railway> railway)
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
    editor->setRailway(railway);
}

void RailDBWindow::afterResetDB()
{
    navi->refreshData();
    editor->setRailway(nullptr);
    editor->refreshData();
}

