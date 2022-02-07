#include "raildbcontext.h"
#include "raildbnavi.h"
#include "raildbwindow.h"
#include "raildb.h"
#include "data/common/qesystem.h"
#include "mainwindow/mainwindow.h"
#include <DockWidget.h>
#include <DockManager.h>
#include <SARibbonContextCategory.h>
#include <SARibbonBar.h>
#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <chrono>
#include "railnet/path/quickpathselector.h"
#include "railnet/path/railpreviewdialog.h"
#include "railnet/graph/viewadjacentwidget.h"
#include "wizards/selectpath/selectpathwizard.h"
#include "util/selectrailwaystable.h"

RailDBContext::RailDBContext(SARibbonContextCategory *cont, MainWindow *mw_):
    QObject(mw_), cont(cont),mw(mw_),
    window(new RailDBWindow(mw_)),_raildb(window->railDB())
{
    connect(window->getNavi(), &RailDBNavi::deactivated,
        this, &RailDBContext::onWindowDeactivated);
    connect(window->getNavi(), &RailDBNavi::importFromCurrent,
        this, &RailDBContext::actImportFromCurrent);
    initUI();
}

RailDBNavi* RailDBContext::getNavi()
{
    return window->getNavi();
}

void RailDBContext::initUI()
{
    auto* page=cont->addCategoryPage(tr("路网管理"));
    QAction* act;
    SARibbonToolButton* btn;

    auto* panel = page->addPannel(tr("功能"));

    if constexpr (true) {
        // 这里同时初始化RailDBDock
        dbDock = new ads::CDockWidget(tr("线路数据库"));
        mw->getManager()->addDockWidgetTab(ads::CenterDockWidgetArea, dbDock);
        dbDock->setWidget(window);
        dbDock->closeDockWidget();
    }
    act = dbDock->toggleViewAction();
    act->setIcon(QIcon(":/icons/database.png"));
    act->setText(tr("数据库面板"));
    act->setToolTip(tr("线路数据库面板 (Ctrl+H)\n"
        "显示或隐藏线路数据库管理面板。"));
    panel->addLargeAction(act);

    if constexpr (true) {
        quickDock = new ads::CDockWidget(tr("快速切片"));
        mw->getManager()->addDockWidget(ads::LeftDockWidgetArea, quickDock);
        quickDock->closeDockWidget();    
        connect(quickDock, &ads::CDockWidget::viewToggled,
            this, &RailDBContext::onQuickToggled);
    }
    act = quickDock->toggleViewAction();
    act->setIcon(QIcon(":/icons/diagram.png"));
    act->setToolTip(tr("快速径路选择 (Ctrl+J)\n"
        "给出关键点，用最短路算法计算径路，生成切片。"));
    panel->addLargeAction(act);

    act = new QAction(QIcon(":/icons/polyline.png"), tr("经由选择"), this);
    connect(act, &QAction::triggered, this, &RailDBContext::actPathSelector);
    act->setToolTip(tr("经由选择器 (Ctrl+K)\n"
        "弹出向导，从线网中手动选择径路，生成线路及其运行图。"));
    panel->addLargeAction(act);

    panel = page->addPannel(tr("查看"));
    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_FileDialogContentsView),
        tr("邻接表"), this);
    panel->addLargeAction(act);
    act->setToolTip(tr("查看邻接表\n查看当前数据库的有向图模型邻接表"));
    connect(act, &QAction::triggered, this, &RailDBContext::actShowAdj);

    panel = page->addPannel(tr("调整"));
    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_BrowserReload),
        tr("刷新线网"), this);
    panel->addLargeAction(act);
    connect(act, &QAction::triggered, this, &RailDBContext::actRefreshNet);
    act->setToolTip(tr("刷新线网\n从当前线路数据库中重新读取有向图模型。"));


    panel = page->addPannel(tr("线路"));
    act = new QAction(QIcon(":/icons/diagram.png"), tr("导出运行图"), this);
    act->setToolTip(tr("导出到运行图\n将数据库中当前所选线路导出至当前打开的运行图。"));
    panel->addLargeAction(act);
    connect(act, &QAction::triggered, getNavi(), &RailDBNavi::actExportToDiagram);

    act = new QAction(qApp->style()->standardIcon(QStyle::SP_FileIcon), tr("导出文件"), this);
    act->setToolTip(tr("导出到文件\n导出数据库中当前所选线路为单线路运行图文件。"));
    panel->addMediumAction(act);
    connect(act, &QAction::triggered, getNavi(), &RailDBNavi::actExportRailToFile);

    act = new QAction(qApp->style()->standardIcon(QStyle::SP_TrashIcon), tr("删除线路"), this);
    act->setToolTip(tr("删除线路\n从数据库中删除当前所选线路。"));
    panel->addMediumAction(act);
    connect(act, &QAction::triggered, getNavi(), &RailDBNavi::actRemoveRail);

    act = new QAction(QIcon(":/icons/ruler.png"), tr("标尺"), this);
    act->setToolTip(tr("编辑标尺\n编辑数据库所选线路下属标尺。"));
    panel->addMediumAction(act);
    connect(act, &QAction::triggered, getNavi(), &RailDBNavi::actRuler);

    act = new QAction(QIcon(":/icons/forbid.png"), tr("天窗"), this);
    act->setToolTip(tr("编辑天窗\n编辑数据库所选线路下属的天窗。"));
    panel->addMediumAction(act);
    connect(act, &QAction::triggered, getNavi(), &RailDBNavi::actForbid);


    panel = page->addPannel(tr(""));
    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton),
        tr("关闭"), this);
    act->setToolTip(tr("关闭线路数据库\n关闭线路数据库面板，关闭打开的文件，清空数据。\n"
        "下次打开时，需要重新读取文件。"));
    connect(act, &QAction::triggered, this, &RailDBContext::deactivateDB);
    panel->addLargeAction(act);
}

bool RailDBContext::deactiveOnClose()
{
    return window->getNavi()->deactiveOnClose();
}

void RailDBContext::onWindowDeactivated()
{
    mw->ribbonBar()->hideContextCategory(cont);
    if(dbDock)
        dbDock->closeDockWidget();
    if(quickDock)
        quickDock->closeDockWidget();
    if(pathDock)
        pathDock->closeDockWidget();
    if(!net.empty()){
        net.clear();
    }
}

void RailDBContext::activateDB()
{
    if (dbDock->isClosed()) {
        dbDock->toggleView();
    }
    else {
        dbDock->setAsCurrentTab();
    }

    activateBase();

    // test
    //loadNet();
    //QString rep;
    //auto rail = net.sliceByPath({ "成都","南充","万源","安康"}, &rep);
    //qDebug() << rep << Qt::endl;
    //
    //auto* w = new RailStationWidget(mw->_diagram.railCategory(), true, mw);
    //w->setRailway(rail);
    //w->setWindowFlags(Qt::Dialog);
    //w->show();
}

void RailDBContext::deactivateDB()
{
    window->deactive();
}

void RailDBContext::activateQuickSelector()
{
    activateBase();
    if (net.empty()){
        loadNet();
        if (net.empty()){
            QMessageBox::warning(mw,tr("警告"),tr("当前有向图模型为空，无法进行常规经由选择。"
            "但仍可使用强制生成径路的功能。\n"
            "出现此问题，可能是因为当前打开的线路数据库（或默认线路数据库）为空。"
            "可先打开线路数据库管理器 (Ctrl+H)，正确加载后，再执行本功能。"));
        }
    }
    if (quickSelector==nullptr){
        quickSelector=new QuickPathSelector(net);
        connect(quickSelector,&QuickPathSelector::railGenerated,
                this,&RailDBContext::previewRail);
        connect(quickSelector, &QuickPathSelector::showStatus,
            mw, &MainWindow::showStatus);
        quickDock->setWidget(quickSelector);
        quickDock->toggleView(true);
        quickDock->setFloating();
        quickDock->window()->resize(500, 500);
    }
    if (quickDock->isClosed()){
        quickDock->toggleView(true);
    }else{
        quickDock->setAsCurrentTab();
    }
}



void RailDBContext::loadDB()
{
    if (!_active) {
        using namespace std::chrono_literals;
        auto start = std::chrono::system_clock::now();
        window->getNavi()->openDB(SystemJson::instance.default_raildb_file);
        _active = true;
        auto end = std::chrono::system_clock::now();
        int ms = (end - start) / 1ms;
        mw->showStatus(tr("线路数据库初始化完成 用时%1 ms").arg(ms));
    }
}

void RailDBContext::activateBase()
{
    loadDB();
    mw->ribbonBar()->showContextCategory(cont);
}

void RailDBContext::actShowAdj()
{
    if (net.empty()) {
        loadNet();
    }
    auto* dlg = new ViewAdjacentWidget(net, mw);
    dlg->show();
}

void RailDBContext::actRefreshNet()
{
    net.clear();
    net.fromRailCategory(_raildb.get());

    // 这里放后续的刷新工作
}

void RailDBContext::onQuickToggled(bool on)
{
    if (on && quickSelector == nullptr) {
        activateQuickSelector();
    }
}

void RailDBContext::actPathSelector()
{
    activateBase();
    if (net.empty()) {
        loadNet();
        if (net.empty()) {
            QMessageBox::warning(mw, tr("警告"), tr("当前有向图模型为空，无法进行经由选择。"
                "只能使用快速径路生成 (Ctrl+J) 中的强制径路生成功能。\n"
                "出现此问题，可能是因为当前打开的线路数据库（或默认线路数据库）为空。"
                "可先打开线路数据库管理器 (Ctrl+H)，正确加载后，在执行本功能。"));
            return;
        }
    }

    auto* wzd = new SelectPathWizard(net, mw);
    connect(wzd, &SelectPathWizard::railwayConfirmed,
        this, &RailDBContext::exportRailToDiagram);
    wzd->show();
}

void RailDBContext::loadNet()
{
    using namespace std::chrono_literals;
    auto start = std::chrono::system_clock::now();
    net.clear();
    net.fromRailCategory(_raildb.get());
    auto end = std::chrono::system_clock::now();
    mw->showStatus(tr("线网有向图加载完毕  共%1站 用时%2毫秒").arg(net.size())
        .arg((end - start) / 1ms));
}

void RailDBContext::previewRail(std::shared_ptr<Railway> railway, const QString& pathString)
{
    if(!dlgPreview){
        dlgPreview=new RailPreviewDialog(mw);
        connect(dlgPreview,&RailPreviewDialog::railConfirmed,
                this,&RailDBContext::exportRailToDiagram);
    }
    dlgPreview->setRailway(railway, pathString);
    dlgPreview->open();
}

void RailDBContext::actImportFromCurrent(const std::deque<int>& path, int path_count)
{
    bool ok;
    auto rails = SelectRailwaysTable::dlgGetRailways(mw, mw->diagram(), tr("选择线路"),
        tr("请选择要导入到数据库的线路，可多选"), &ok);
    if (!ok)return;
    if (rails.empty()) {
        QMessageBox::warning(mw, tr("错误"), tr("没有选择线路。请选择至少一条线路来导入。"));
        return;
    }

    // 下面：实施导入操作
    for (auto p : rails) {
        p->setName(_raildb->validRailwayNameRec(p->name()));
    }

    auto np = path;
    np.push_back(path_count);
    getNavi()->undoStack()->push(new qecmd::ImportRailsDB(rails, np, getNavi()->getModel()));
}
