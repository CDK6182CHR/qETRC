#include "viewcategory.h"
#include "mainwindow.h"
#include "data/diagram/trainadapter.h"


#include <SARibbonPannelItem.h>
#include <SARibbonGallery.h>
#include <SARibbonGalleryGroup.h>



ViewCategory::ViewCategory(MainWindow *mw_,
                           SARibbonCategory *cat_, QObject *parent):
    QObject(parent),cat(cat_),diagram(mw_->_diagram),mw(mw_)
{
    initUI();
}

void ViewCategory::commitTrainsShow(const QList<std::shared_ptr<TrainLine>>& lines, bool show)
{
    for (auto line : lines) {
        line->setIsShow(show);
        setTrainShow(line, show);
    }
    onTrainShowChanged();
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

    act = new QAction(QIcon(":/icons/refresh.png"), tr("刷新类型表"), this);
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(80);
    connect(act, SIGNAL(triggered()), this, SLOT(refreshTypeGroup()));

    act = new QAction(tr("客车类型"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(selectPassengers()));
    panel->addMediumAction(act);

    act = new QAction(tr("反向选择"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(selectReversed()));
    panel->addMediumAction(act);

    act = new QAction(QIcon(":/icons/tick.png"), tr("应用"), this);
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
}

void ViewCategory::setDirTrainsShow(Direction dir, bool show)
{
    QList<std::shared_ptr<TrainLine>> lines;
    const auto& mana = diagram.trainCollection().typeManager();

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

void ViewCategory::setTrainShow(std::shared_ptr<TrainAdapter> adp, bool show)
{
    for (auto p : mw->diagramWidgets)
        p->setTrainShow(adp, show);
}

bool ViewCategory::typeIsShow(std::shared_ptr<Train> train) const
{
    return !diagram.config().not_show_types.contains(train->type()->name());
}

void ViewCategory::onTrainShowChanged()
{
    //TODO: 暂定暴力更新
    mw->trainListWidget->refreshData();
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

void ViewCategory::refreshTypeGroup()
{
    auto& coll = diagram.trainCollection();
    const TypeManager& manager = coll.typeManager();
    auto* model = group->groupModel();
    auto& cfg = diagram.config();
    model->clear();
    int row = 0;
    for (auto p = manager.types().begin(); p != manager.types().end(); ++p) {
        auto* act = new QAction(p.key(), this);
        auto* item = new SARibbonGalleryItem(act);
        model->append(item);
        if (!cfg.not_show_types.contains(p.key())) {
            //选择
            group->selectionModel()->select(model->index(row, 0, QModelIndex()),
                QItemSelectionModel::Select);
        }
        row++;
    }
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
    QUndoCommand(QObject::tr("设置显示类型"), parent), lines(lines_),cat(cat_), 
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
