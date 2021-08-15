#include "rulercontext.h"
#include "mainwindow.h"

RulerContext::RulerContext(Diagram& diagram_, SARibbonContextCategory *context, MainWindow *mw_):
    QObject(mw_),diagram(diagram_), cont(context),mw(mw_)
{
    initUI();
}

void RulerContext::setRuler(std::shared_ptr<Ruler> ruler)
{
    this->ruler=ruler;
    refreshData();
}

void RulerContext::refreshData()
{
    if (!ruler)return;
    edRulerName->setText(ruler->name());
    edRailName->setText(ruler->railway().name());
}

void RulerContext::refreshAllData()
{
    refreshData();
}

void RulerContext::initUI()
{
    auto* page = cont->addCategoryPage(tr("标尺"));
    auto* panel = page->addPannel(tr("当前标尺"));

    auto* ed = new QLineEdit;
    ed->setFocusPolicy(Qt::NoFocus);
    ed->setAlignment(Qt::AlignCenter);
    ed->setFixedWidth(150);
    ed->setToolTip(tr("当前标尺名称\n不可编辑；如需编辑请至编辑面板操作"));
    edRulerName = ed;
    panel->addWidget(ed, SARibbonPannelItem::Medium);

    ed = new QLineEdit;
    ed->setFocusPolicy(Qt::NoFocus);
    ed->setAlignment(Qt::AlignCenter);
    ed->setToolTip(tr("当前标尺所属线路"));
    edRailName = ed;
    ed->setFixedWidth(150);
    panel->addWidget(ed, SARibbonPannelItem::Medium);

    panel = page->addPannel(tr("编辑"));
    auto* act = new QAction(QIcon(":/icons/edit.png"), tr("编辑"), this);
    act->setToolTip(tr("标尺编辑面板\n显示当前标尺的编辑面板，编辑标尺名称和数据。"));
    connect(act, SIGNAL(triggered()), this, SLOT(actShowEditWidget()));
    auto* btn = panel->addLargeAction(act);
    btn->setMinimumWidth(70);

    //todo: 标尺推导；从车次读取等

    panel = page->addPannel(tr("设置"));
    act = new QAction(QIcon(":/icons/ruler.png"), tr("排图标尺"), this);
    act->setToolTip(tr("设为排图标尺\n将当前标尺设置为本线的排图标尺，即作为纵坐标标度"));
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(80);
    connect(act, SIGNAL(triggered()), this, SLOT(actSetAsOrdinate()));

    act = new QAction(QIcon(":/icons/close.png"), tr("删除标尺"), this);
    act->setToolTip(tr("删除标尺\n删除当前标尺。如果当前标尺同时是排图标尺，"
        "则同时将本线设置为按里程排图。"));
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(70);
    connect(act, SIGNAL(triggered()), this, SLOT(actRemoveRuler()));
    
}

void RulerContext::actChangeRulerData(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> nr)
{
    mw->getUndoStack()->push(new qecmd::UpdateRuler(ruler, nr, this));
}

void RulerContext::commitRulerChange(std::shared_ptr<Ruler> ruler)
{
    mw->getRailContext()->refreshRulerTable(ruler);
    if (ruler->isOrdinateRuler()) {
        emit ordinateRulerModified(ruler->railway());
    }
}

void RulerContext::actChangeRulerName(std::shared_ptr<Ruler> ruler, const QString& name)
{
    mw->getUndoStack()->push(new qecmd::ChangeRulerName(ruler, name, this));
}

void RulerContext::commitChangeRulerName(std::shared_ptr<Ruler> ruler)
{
    if (ruler == this->ruler) {
        refreshData();
    }
    mw->getRailContext()->onRulerNameChanged(ruler);
}

void RulerContext::commitRemoveRuler(std::shared_ptr<Ruler> ruler, bool isord)
{
    // 实施删除标尺：从线路中移除数据；退出当前面板；如果是ord则撤销ord
    auto& rail = ruler->railway();

    mw->getRailContext()->removeRulerAt(ruler->railway(), ruler->index(), isord);
    mw->getRailContext()->removeRulerWidget(ruler);

    if (isord) {
        //重新排图
        mw->updateRailwayDiagrams(rail);
    }

    if(ruler==this->ruler)
        emit focusOutRuler();
}

void RulerContext::undoRemoveRuler(std::shared_ptr<Ruler> ruler, bool isord)
{
    if (isord) {
        mw->updateRailwayDiagrams(ruler->railway());
    }
    mw->getRailContext()->insertRulerAt(ruler->railway(), ruler, isord);
}

void RulerContext::actSetAsOrdinate()
{
    mw->getRailContext()->actChangeOrdinate(ruler->index() + 1);
}

void RulerContext::actRemoveRuler()
{
    auto r = ruler->clone();
    mw->getUndoStack()->push(new qecmd::RemoveRuler(ruler, r, ruler->isOrdinateRuler(),
        this));
}

void RulerContext::actShowEditWidget()
{
    mw->getRailContext()->openRulerWidget(ruler);
}

void qecmd::UpdateRuler::undo()
{
    ruler->swap(*(nr->getRuler(0)));
    cont->commitRulerChange(ruler);
}

void qecmd::UpdateRuler::redo()
{
    ruler->swap(*(nr->getRuler(0)));
    cont->commitRulerChange(ruler);
}

void qecmd::ChangeRulerName::undo()
{
    std::swap(ruler->nameRef(), name);
    cont->commitChangeRulerName(ruler);
}

void qecmd::ChangeRulerName::redo()
{
    std::swap(ruler->nameRef(), name);
    cont->commitChangeRulerName(ruler);
}

void qecmd::RemoveRuler::undo()
{
    ruler->railway().undoRemoveRuler(ruler, data);
    if (isOrd) {
        ruler->railway().setOrdinate(ruler);
    }
    cont->undoRemoveRuler(ruler, isOrd);
}

void qecmd::RemoveRuler::redo()
{
    ruler->railway().removeRuler(ruler);
    cont->commitRemoveRuler(ruler, isOrd);
}
