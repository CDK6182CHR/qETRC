﻿#ifndef QETRC_MOBILE_2
#include "viewcategory.h"
#include "mainwindow.h"
#include "data/diagram/trainadapter.h"
#include "editors/configdialog.h"
#include "editors/typeconfigdialog.h"
#include "editors/typeregexdialog.h"
#include "data/train/traintype.h"
#include "editors/systemjsondialog.h"
#include "dialogs/trainfilter.h"
#include "editors/trainlistwidget.h"
#include "model/train/trainlistmodel.h"
#include "data/diagram/config.h"
#include "data/common/qesystem.h"
#include "data/diagram/diagrampage.h"

#include <SARibbonPannelItem.h>
#include <SARibbonGallery.h>
#include <SARibbonGalleryGroup.h>
#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <SARibbonMenu.h>
#include <SARibbonCategory.h>


ViewCategory::ViewCategory(MainWindow *mw_,
                           SARibbonCategory *cat_, QObject *parent):
    QObject(parent),cat(cat_),diagram(mw_->_diagram),mw(mw_),
    filter(new TrainFilter(mw_->_diagram,mw))
{
    initUI();
    connect(filter, &TrainFilter::filterApplied,
        this, &ViewCategory::trainFilterApplied);
}

void ViewCategory::commitTrainsShow(const QList<std::shared_ptr<TrainLine>>& lines, bool show)
{
    foreach (auto line , lines) {
        line->setIsShow(show);
        setTrainShow(line, show);
    }
    onTrainShowChanged();
}

void ViewCategory::commitTrainsShowByFilter(const QVector<std::shared_ptr<TrainLine>>& lines)
{
    foreach(auto line, lines) {
        line->setIsShow(!line->show());
        setTrainShow(line, line->show());
    }
    onTrainShowChanged();
}

void ViewCategory::commitSingleTrainShow(const QList<std::shared_ptr<TrainLine>>& lines, bool show)
{
    for (auto line : lines) {
        line->setIsShow(show);
        setTrainShow(line, show);
    }
}

void ViewCategory::commitTypeShow(const QList<std::shared_ptr<TrainLine>>& lines)
{
    for (auto line : lines) {
        line->setIsShow(!line->show());
        setTrainShow(line, line->show());
    }
    refreshTypeGroup();
    onTrainShowChanged();
}

void ViewCategory::initUI()
{
    auto* panel = cat->addPannel(tr("行别显示控制"));

    rdDirType = new RadioButtonGroup<2, QVBoxLayout>({ "使用入图行别","使用出图行别" }, cat);
    rdDirType->get(0)->setChecked(true);
    QWidget* w = new QWidget;
    w->setLayout(rdDirType);
    panel->addWidget(w, SARibbonPannelItem::Large);

    QAction* act = new QAction(QIcon(":/icons/diagram.png"), tr("运行线级别控制"), this);
    act->setCheckable(true);
    actLineCtrl = act;
    connect(act, SIGNAL(triggered(bool)), this, SLOT(lineControlTriggered(bool)));
    auto* btn = panel->addLargeAction(act);

    act = new QAction(tr("显示下行"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(showDown()));
    panel->addMediumAction(act);

    act = new QAction(tr("隐藏下行"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(hideDown()));
    panel->addMediumAction(act);


    act = new QAction(tr("显示上行"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(showUp()));
    panel->addMediumAction(act);

    act = new QAction(tr("隐藏上行"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(hideUp()));
    panel->addMediumAction(act);

    //类型显示控制
    panel = cat->addPannel(tr("类型显示控制"));

    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_BrowserReload),
        tr("刷新类型表"), this);
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(80);
    connect(act, SIGNAL(triggered()), this, SLOT(refreshTypeGroup()));

    act = new QAction(tr("客车类型"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(selectPassengers()));
    panel->addMediumAction(act);

    act = new QAction(tr("反向选择"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(selectReversed()));
    panel->addMediumAction(act);

    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton),
        tr("应用"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(applyTypeShow()));
    panel->addLargeAction(act);

    gall = panel->addGallery();
    group = gall->addGalleryGroup();
    group->setSelectionMode(QAbstractItemView::MultiSelection);
    //group->setSelectionBehavior(QAbstractItemView::SelectItems);
    gall->currentViewGroup()->setSelectionMode(QAbstractItemView::MultiSelection);
    gall->currentViewGroup()->setSelectionModel(group->selectionModel());
    group->setStyleSheet("QListView::item{height:30px;}");
    group->setGridSize(QSize(group->gridSize().width(), SystemJson::instance.table_row_height));
    gall->currentViewGroup()->setGridSize(QSize(group->gridSize().width(),
        SystemJson::instance.table_row_height));

    act = new QAction(QIcon(":/icons/filter.png"), tr("高级"), this);
    act->setToolTip(tr("高级显示类型筛选 (Ctrl+Shift+L)\n"
        "使用（与pyETRC一致的）车次筛选器来决定显示车次的集合。"));
    act->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_L);
    mw->addAction(act);
    connect(act, &QAction::triggered, filter, &TrainFilter::show);
    panel->addLargeAction(act);

    panel = cat->addPannel(tr("设置"));
    act = new QAction(QIcon(":/icons/config.png"), tr("显示设置"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(actShowConfig()));

    auto* m = new SARibbonMenu(cat);
    auto* ma = m->addAction(tr("系统默认显示设置"));
    connect(ma, &QAction::triggered, this, &ViewCategory::actShowDefaultConfig);

    ma = m->addAction(tr("使用系统默认设置"));
    connect(ma, &QAction::triggered, this, &ViewCategory::actApplyDefaultConfig);

    ma = m->addAction(tr("保存当前运行图设置为默认"));
    connect(ma, &QAction::triggered, this, &ViewCategory::actSaveConfigAsDefault);

    m->addSeparator();
    m->addAction(tr("将显示设置应用到所有运行图页面"), this, &ViewCategory::actApplyConfigToPages);

    m->addSeparator();
    ma = m->addAction(tr("全局配置选项"));
    connect(ma, &QAction::triggered, this, &ViewCategory::actSystemJsonDialog);
    act->setMenu(m);

    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(80);
    

    act = new QAction(QIcon(":/icons/settings.png"), tr("类型管理"), this);
    connect(act, &QAction::triggered, this, &ViewCategory::actTypeConfig);
    act->setToolTip(tr("类型管理\n管理[当前运行图文件]的列车类型，及各种类型的颜色、线形等。"));
    panel->addMediumAction(act);

    m = new SARibbonMenu(cat);
    ma = m->addAction(tr("默认类型管理"));
    connect(ma, &QAction::triggered, this, &ViewCategory::actTypeConfigDefault);
    ma->setToolTip(tr("默认类型管理\n配置[系统默认设置]的类型管理部分，"
        "将用于缺省的新运行图。"));
    act->setMenu(m);

    act = new QAction(QIcon(":/icons/filter.png"), tr("类型规则"), this);
    connect(act, &QAction::triggered, this, &ViewCategory::actTypeRegex);
    act->setToolTip(tr("类型判定规则\n编辑[当前运行图文件]的由正则表达式判定车次所属类型的规则。"));
    panel->addMediumAction(act);

    m = new SARibbonMenu(cat);
    ma = m->addAction(tr("默认类型规则"));
    connect(ma, &QAction::triggered, this, &ViewCategory::actTypeRegexDefault);
    ma->setToolTip(tr("默认类型规则\n配置[系统默认设置]的正则表达式判定车次所述类型规则，"
        "将用于缺省的新运行图。"));
    act->setMenu(m);
}

void ViewCategory::setDirTrainsShow(Direction dir, bool show)
{
    QList<std::shared_ptr<TrainLine>> lines;

    auto addline = [&lines, show, this](std::shared_ptr<TrainLine> line,
        std::shared_ptr<Train> train) {
            if (line->show() != show && (!show || typeIsShow(train))) {
                lines.append(line);
            }
    };

    auto addadp = [&lines, show, this, addline](std::shared_ptr<TrainAdapter> adp,
        std::shared_ptr<Train> train) {
        for (auto p : adp->lines()) {
            addline(p, train);
        }
    };

    for (auto train : diagram.trainCollection().trains()) {
        for (auto adp : train->adapters()) {
            if (actLineCtrl->isChecked()) {
                for (auto line : adp->lines()) {
                    if(line->dir()==dir)
                        addline(line, train);
                }
            }
            else {
                if (rdDirType->get(0)->isChecked()) {
                    //用入图行别判定
                    if (adp->firstDirection() == dir)
                        addadp(adp, train);
                }
                else {
                    //出图行别
                    if (adp->lastDirection() == dir)
                        addadp(adp, train);
                }
            }
        }
    }
    if(!lines.empty())
        mw->undoStack->push(new qecmd::ChangeTrainShow(lines, show, this));
}

void ViewCategory::setTrainShow(std::shared_ptr<TrainLine> line, bool show)
{
    for (auto p : mw->diagramWidgets)
        p->setTrainShow(line, show);
}

bool ViewCategory::typeIsShow(std::shared_ptr<Train> train) const
{
    return !diagram.config().not_show_types.contains(train->type()->name());
}

void ViewCategory::onTrainShowChanged()
{
    mw->trainListWidget->getModel()->updateAllTrainShow();
}

void ViewCategory::showDown()
{
    setDirTrainsShow(Direction::Down, true);
}

void ViewCategory::showUp()
{
    setDirTrainsShow(Direction::Up, true);
}

void ViewCategory::hideDown()
{
    setDirTrainsShow(Direction::Down, false);
}

void ViewCategory::hideUp()
{
    setDirTrainsShow(Direction::Up, false);
}

void ViewCategory::lineControlTriggered(bool on)
{
    for (int i = 0; i < 2; i++) {
        rdDirType->get(i)->setEnabled(!on);
    }
}

void ViewCategory::selectPassengers()
{
    auto* model = group->groupModel();
    auto* am = group->model();
    auto* sel = group->selectionModel();
    auto& ma = diagram.trainCollection().typeManager();
    for (int i = 0; i < am->rowCount(); i++) {
        const QString& name = model->at(i)->text();
        auto tp = ma.find(name);
        if (tp && tp->isPassenger())
            sel->select(model->index(i, 0, QModelIndex()), QItemSelectionModel::Select);
        else
            sel->select(model->index(i, 0, QModelIndex()), QItemSelectionModel::Deselect);
    }
}

void ViewCategory::selectReversed()
{
    auto* sel = group->selectionModel();
    auto* model = group->groupModel();
    for (int i = 0; i < model->rowCount({}); i++) {
        sel->select(model->index(i, 0, {}), QItemSelectionModel::Toggle);
    }
}

void ViewCategory::applyTypeShow()
{
    //用model
    auto* model = group->groupModel();
    auto* sel = group->selectionModel();
    QSet<QString> notShow;
    QList<std::shared_ptr<TrainLine>> lines;

    auto addfunc = [&lines](std::shared_ptr<Train> train, bool show) {
        for (auto adp : train->adapters()) {
            for (auto line : adp->lines()) {
                if (line->show() != show) {
                    line->setIsShow(!show);   //这是为了同步状态
                    lines.append(line);
                }
            }
        }
    };

    for (int i = 0; i < model->rowCount(QModelIndex()); i++) {
        if (!sel->isSelected(model->index(i, 0, QModelIndex()))) {
            notShow.insert(model->at(i)->text());
        }
    }
    //设置列车的显示情况
    for (auto train : diagram.trainCollection().trains()) {
        bool show = !(notShow.contains(train->type()->name()));
        addfunc(train, show);
    }

    if (notShow != diagram.config().not_show_types || !lines.empty()) {
        //非平凡操作，压栈执行
        mw->undoStack->push(new qecmd::ChangeTypeShow(lines, this, diagram.config(), notShow));
    }
}

void ViewCategory::actShowConfig()
{
    if (informConfig) {
        QMessageBox::information(mw, tr("提示"), tr("自1.0.1版本开始，每个运行图页面（DiagramPage）"
            "可以设置不同的运行图显示。此处的设定只决定此后新增的页面的默认设置情况，"
            "不会影响既有运行图的设置。如需调整既有运行图页面的显示设置，"
            "请至对应运行图页面的上下文菜单，或将当前显示设置应用到所有页面。\n"
            "此提示在程序每次运行期间，展示一次。"));
        informConfig = false;
    }
    auto* dialog = new ConfigDialog(diagram.config(), false, mw);
    connect(dialog, &ConfigDialog::onConfigApplied, this,
        &ViewCategory::onActConfigApplied);
    dialog->open();
}

void ViewCategory::actShowDefaultConfig()
{
    auto* dialog = new ConfigDialog(diagram.defaultConfig(), true, mw);
    connect(dialog, &ConfigDialog::onConfigApplied, this,
        &ViewCategory::onActConfigApplied);
    dialog->open();
}

void ViewCategory::onActConfigApplied(Config& cfg, const Config& newcfg, bool repaint,
    bool forDefault)
{
    mw->getUndoStack()->push(new qecmd::ChangeConfig(cfg, newcfg, repaint, forDefault, this));
}

void ViewCategory::actApplyConfigToPages()
{
    auto flag = QMessageBox::question(mw, tr("提示"), tr("此操作将当前的运行图显示设置"
        "应用到所有现存的运行图页面（DiagramPage）。是否确认？"));
    if (flag != QMessageBox::Yes)
        return;
    auto* cmd = new qecmd::ApplyConfigToPages;
    foreach(auto page, diagram.pages()) {
        new qecmd::ChangePageConfig(page->configRef(),
            diagram.config(), true, page, this, cmd);
    }
    mw->getUndoStack()->push(cmd);
}

void ViewCategory::actTypeConfig()
{
    auto* dialog = new TypeConfigDialog(diagram.trainCollection().typeManager(),false, mw);
    connect(dialog, &TypeConfigDialog::typeSetApplied,
        this, &ViewCategory::actCollTypeSetChanged);
    dialog->show();
}

void ViewCategory::actTypeConfigDefault()
{
    auto* dialog = new TypeConfigDialog(diagram.defaultTypeManager(), true, mw);
    connect(dialog, &TypeConfigDialog::typeSetApplied,
        this, &ViewCategory::actDefaultTypeSetChanged);
    dialog->show();
}

void ViewCategory::actTypeRegex()
{
    auto* dialog = new TypeRegexDialog(diagram.trainCollection().typeManager(),false, mw);
    connect(dialog, &TypeRegexDialog::typeRegexApplied,
        this, &ViewCategory::actCollTypeRegexChanged);
    dialog->show();
}

void ViewCategory::actTypeRegexDefault()
{
    auto* dialog = new TypeRegexDialog(diagram.defaultTypeManager(), true, mw);
    connect(dialog, &TypeRegexDialog::typeRegexApplied,
        this, &ViewCategory::actDefaultTypeRegexChanged);
    dialog->show();
}

void ViewCategory::actSystemJsonDialog()
{
    auto* dialog = new SystemJsonDialog(mw);
    dialog->show();
}

void ViewCategory::actApplyDefaultConfig()
{
    auto flag = QMessageBox::question(mw, tr("提示"), tr("使用系统默认设置：将系统默认设置"
        "应用到当前运行图设置。是否确认？"));
    if (flag == QMessageBox::Yes) {
        mw->getUndoStack()->push(new qecmd::ChangeConfig(diagram.config(),
            diagram.defaultConfig(), true, false, this));
    }
}

void ViewCategory::actSaveConfigAsDefault()
{
    auto flag = QMessageBox::question(mw, tr("提示"), tr("保存当前设置为默认：将当前运行图"
        "的设置保存为系统默认设置。是否继续？"));
    if (flag == QMessageBox::Yes) {
        mw->getUndoStack()->push(new qecmd::ChangeConfig(diagram.defaultConfig(),
            diagram.config(), false, true, this));
    }
}

void ViewCategory::trainFilterApplied()
{
    QVector<std::shared_ptr<TrainLine>> lines;
    foreach(auto train, diagram.trainCollection().trains()) {
        bool show = filter->check(train);
        //qDebug() << train->trainName().full() << ", " << show;
        foreach(auto adp, train->adapters()) {
            foreach(auto line, adp->lines()) {
                if (line->show() != show) {
                    lines.append(line);
                }
            }
        }
    }
    if(!lines.empty())
        mw->getUndoStack()->push(new qecmd::ChangeTrainsShowByFilter(std::move(lines), this));
}

void ViewCategory::commitConfigChange(Config& cfg, bool repaint)
{
    Q_UNUSED(cfg);
    Q_UNUSED(repaint);
    // 2021.10.08：不需要重绘了
    //mw->updateAllDiagrams();
}

void ViewCategory::commitPageConfigChange(std::shared_ptr<DiagramPage> page, bool repaint)
{
    Q_UNUSED(repaint)
    mw->updatePageDiagram(page);
}

void ViewCategory::actChangeSingleTrainShow(std::shared_ptr<Train> train, bool show)
{
    QList<std::shared_ptr<TrainLine>> lines;
    foreach(auto p, train->adapters()) {
        foreach(auto line, p->lines()) {
            if (line->show() != show) {
                lines.push_back(line);
            }
        }
    }
    if (!lines.empty()) {
        mw->getUndoStack()->push(new qecmd::ChangeSingleTrainShow(train, show, lines, this));
    }
}

void ViewCategory::onActPageConfigApplied(Config& cfg, const Config& newcfg, 
    bool repaint, std::shared_ptr<DiagramPage> page)
{
    mw->getUndoStack()->push(new qecmd::ChangePageConfig(cfg, newcfg, repaint, page, this));
}

void ViewCategory::refreshTypeGroup()
{
    auto& coll = diagram.trainCollection();
    coll.refreshTypeCount();
    auto* model = group->groupModel();
    auto& cfg = diagram.config();
    model->clear();
    int row = 0;
    for (auto p = coll.typeCount().cbegin(); p != coll.typeCount().cend(); ++p) {
        if (p.value() <= 0)
            continue;
        auto* act = new QAction(p.key()->name(), this);
        auto* item = new SARibbonGalleryItem(act);
        model->append(item);
        if (!cfg.not_show_types.contains(p.key()->name())) {
            //选择
            group->selectionModel()->select(model->index(row, 0, QModelIndex()),
                QItemSelectionModel::Select);
        }
        row++;
    }
}

void ViewCategory::actCollTypeSetChanged(TypeManager& manager, 
    const QMap<QString, std::shared_ptr<TrainType>>& types, 
    const QVector<QPair<std::shared_ptr<TrainType>, std::shared_ptr<TrainType>>>& modified)
{
    if (&manager != &(diagram.trainCollection().typeManager())) {
        qDebug() << "ViewCategory::actCollTypeSetChanged: incompatible manager! nothing todo."
            << Qt::endl;
        return;
    }

    QSet<std::shared_ptr<TrainType>> typesInSet;
    foreach(auto t, types) {
        typesInSet.insert(t);
    }

    auto typesCopy = types;    // copy construct

    foreach(auto train, diagram.trainCollection().trains()) {
        auto tp = train->type();
        if (!tp) {
            qDebug() << "ViewCategory::actCollTypeSetChanged: WARNING: " <<
                "train type is null! " << train->trainName().full() << Qt::endl;
        }
        else {
            if (!typesInSet.contains(tp)) {
                qDebug() << "ViewCategory::actCollTypeSetChanged: INFO: " <<
                    "add type " << tp->name() << " to set. " << Qt::endl;
                typesInSet.insert(tp);
                typesCopy.insert(tp->name(), tp);
            }
        }
    }

    mw->getUndoStack()->push(new qecmd::ChangeTypeSet(manager,
        typesCopy, modified, this, false));
}

void ViewCategory::actDefaultTypeSetChanged(TypeManager& manager,
    const QMap<QString, std::shared_ptr<TrainType>>& types,
    const QVector<QPair<std::shared_ptr<TrainType>, std::shared_ptr<TrainType>>>& modified)
{
    mw->getUndoStack()->push(new qecmd::ChangeTypeSet(manager, types, modified, this, true));
}


void ViewCategory::actCollTypeRegexChanged(TypeManager& manager,
    std::shared_ptr<TypeManager> data)
{
    mw->getUndoStack()->push(new qecmd::ChangeTypeRegex(manager, data,this,false));
}

void ViewCategory::actDefaultTypeRegexChanged(TypeManager& manager, std::shared_ptr<TypeManager> data)
{
    mw->getUndoStack()->push(new qecmd::ChangeTypeRegex(manager, data,this,true));
}

void ViewCategory::saveDefaultConfigs()
{
    mw->saveDefaultConfig();
}

qecmd::ChangeTrainShow::ChangeTrainShow(const QList<std::shared_ptr<TrainLine>>& lines_,
    bool show_, ViewCategory* cat_, QUndoCommand* parent):
    QUndoCommand(parent),lines(lines_),show(show_),cat(cat_)
{
    if (show) {
        setText(QObject::tr("显示%1条运行线").arg(lines.size()));
    }
    else {
        setText(QObject::tr("隐藏%1条运行线").arg(lines.size()));
    }
}

void qecmd::ChangeTrainShow::undo()
{
    cat->commitTrainsShow(lines, !show);
}

void qecmd::ChangeTrainShow::redo()
{
    cat->commitTrainsShow(lines, show);
}

qecmd::ChangeTypeShow::ChangeTypeShow(const QList<std::shared_ptr<TrainLine>>& lines_, 
    ViewCategory* cat_, Config& cfg_, 
    QSet<QString> notShowTypes_, QUndoCommand* parent):
    QUndoCommand(QObject::tr("设置显示类型 影响%1条运行线").arg(lines_.size()), parent),
    lines(lines_),cat(cat_), 
    cfg(cfg_), notShowTypes(notShowTypes_)
{
}

void qecmd::ChangeTypeShow::undo()
{
    std::swap(cfg.not_show_types, notShowTypes);
    cat->commitTypeShow(lines);
}

void qecmd::ChangeTypeShow::redo()
{
    std::swap(cfg.not_show_types, notShowTypes);
    cat->commitTypeShow(lines);
}

qecmd::ChangeSingleTrainShow::ChangeSingleTrainShow(std::shared_ptr<Train> train_, 
    bool show_, const QList<std::shared_ptr<TrainLine>>& lines_, 
    ViewCategory* cat_, QUndoCommand* parent):
   QUndoCommand(parent),train(train_),show(show_),lines(lines_),cat(cat_)
{
    QString s = (show ? QObject::tr("显示") : QObject::tr("隐藏"));
    setText(QObject::tr("%1列车运行线: %2").arg(s, train->trainName().full()));
}

void qecmd::ChangeSingleTrainShow::undo()
{
    train->setIsShow(!show);
    cat->commitSingleTrainShow(lines, !show);
}

void qecmd::ChangeSingleTrainShow::redo()
{
    train->setIsShow(show);
    cat->commitSingleTrainShow(lines, show);
}

void qecmd::ChangeTypeSet::undo()
{
    commit();
}

void qecmd::ChangeTypeSet::redo()
{
    commit();
}

void qecmd::ChangeTypeSet::commit()
{
    foreach(auto & p, modified) {
        p.first->swap(*p.second);
    }
    std::swap(manager.typesRef(), types);
    if (forDefault) cat->saveDefaultConfigs();
}

void qecmd::ChangeTypeRegex::undo()
{
    manager.swapForRegex(*data);
    if (forDefault) cat->saveDefaultConfigs();
}

void qecmd::ChangeTypeRegex::redo()
{
    manager.swapForRegex(*data);
    if (forDefault) cat->saveDefaultConfigs();
}

void qecmd::ChangeTrainsShowByFilter::undo()
{
    cat->commitTrainsShowByFilter(lines);
}
void qecmd::ChangeTrainsShowByFilter::redo()
{
    cat->commitTrainsShowByFilter(lines);
}

qecmd::ApplyConfigToPages::ApplyConfigToPages(QUndoCommand* parent):
    QUndoCommand(parent)
{
    setText(QObject::tr("应用显示设置至运行图页面"));
}

#endif
