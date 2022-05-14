#ifndef QETRC_MOBILE_2
#include "rulercontext.h"
#include "mainwindow.h"
#include "railcontext.h"

#include "dialogs/rulerfromtraindialog.h"
#include "dialogs/rulerfromspeeddialog.h"
#include "util/railrulercombo.h"
#include <QApplication>
#include <QStyle>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>

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
    edRailName->setText(ruler->railway()->name());
}

void RulerContext::refreshAllData()
{
    refreshData();
}

void RulerContext::initUI()
{
    auto* page = cont->addCategoryPage(tr("标尺管理(&9)"));
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

    act = new QAction(QIcon(":/icons/identify.png"), tr("从车次提取"), this);
    act->setToolTip(tr("从单车次提取标尺\n"
        "从单个车次在本线的运行情况提取标尺数据，并覆盖到本标尺中。"));
    connect(act, SIGNAL(triggered()), this, SLOT(actReadFromSingleTrain()));
    btn = panel->addMediumAction(act);

    act = new QAction(QIcon(":/icons/clock.png"), tr("从速度计算"), this);
    act->setToolTip(tr("从运行速度计算\n"
        "从通通运行速度（不含起停附加时分）计算近似标尺，并覆盖到本标尺中。"));
    connect(act, &QAction::triggered, this, &RulerContext::actFromSpeed);
    panel->addMediumAction(act);

    act = new QAction(qApp->style()->standardIcon(QStyle::SP_ArrowBack), tr("合并标尺"), this);
    act->setToolTip(tr("合并标尺\n与本线路的其他标尺合并。"));
    connect(act, &QAction::triggered, this, &RulerContext::mergeCurrent);
    panel->addMediumAction(act);

    panel = page->addPannel(tr("设置"));
    act = new QAction(QIcon(":/icons/ruler.png"), tr("排图标尺"), this);
    act->setToolTip(tr("设为排图标尺\n将当前标尺设置为本线的排图标尺，即作为纵坐标标度"));
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(80);
    connect(act, SIGNAL(triggered()), this, SLOT(actSetAsOrdinate()));

    act = new QAction(QIcon(":/icons/copy.png"), tr("副本"), this);
    act->setToolTip(tr("创建标尺副本\n使用输入的标尺名称，在本线路下创建当前标尺副本"));
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(80);
    connect(act, &QAction::triggered, this, &RulerContext::actDulplicateRuler);

    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_TrashIcon),
        tr("删除标尺"), this);
    act->setToolTip(tr("删除标尺\n删除当前标尺。如果当前标尺同时是排图标尺，"
        "则同时将本线设置为按里程排图。"));
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(70);
    connect(act, SIGNAL(triggered()), this, SLOT(actRemoveRuler()));

    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton),
        tr("关闭面板"), this);
    act->setToolTip(tr("关闭面板\n关闭当前的标尺上下文工具栏页面"));
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(70);
    connect(act, &QAction::triggered, this, &RulerContext::focusOutRuler);
    
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
    emit rulerNameChanged(ruler);
}

void RulerContext::commitRemoveRuler(std::shared_ptr<Ruler> ruler, bool isord)
{
    // 实施删除标尺：从线路中移除数据；退出当前面板；如果是ord则撤销ord
    auto rail = ruler->railway();

    mw->getRailContext()->removeRulerAt(*(ruler->railway()), ruler->index(), isord);
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
    mw->getRailContext()->insertRulerAt(*(ruler->railway()), ruler, isord);
}

void RulerContext::actRemoveRulerNavi(std::shared_ptr<Ruler> ruler)
{
    auto r = ruler->clone();
    mw->getUndoStack()->push(new qecmd::RemoveRuler(ruler, r, ruler->isOrdinateRuler(),
        this));
}



void RulerContext::actSetAsOrdinate()
{
    if (mw->getRailContext()->getRailway()==(ruler->railway()))
        mw->getRailContext()->actChangeOrdinate(ruler->index() + 1);
    else {
        // 需要这里来设置Ordinate -- 操作压栈
        mw->getRailContext()->changeRailOrdinate(ruler->railway(), ruler->index());
    }
}

void RulerContext::actRemoveRuler()
{
    auto r = ruler->clone();
    mw->getUndoStack()->push(new qecmd::RemoveRuler(ruler, r, ruler->isOrdinateRuler(),
        this));
}

void RulerContext::actReadFromSingleTrain()
{
    if (!ruler)
        return;
    auto* dialog = new RulerFromTrainDialog(diagram.trainCollection(), ruler, mw);
    connect(dialog, &RulerFromTrainDialog::rulerUpdated,
        this, &RulerContext::actChangeRulerData);
    dialog->show();
}

void RulerContext::actFromSpeed()
{
    if (!ruler)return;
    auto* dialog = new RulerFromSpeedDialog(ruler, mw);
    connect(dialog, &RulerFromSpeedDialog::rulerUpdated,
        this, &RulerContext::actChangeRulerData);
    dialog->show();
}

void RulerContext::dulplicateRuler(std::shared_ptr<Ruler> ruler)
{
    if (!ruler)
        return;
    auto rail = ruler->railway();

    bool ok;
    auto name = QInputDialog::getText(mw, tr("创建标尺副本"), tr("现在创建基线[%1]中标尺[%2]的副本。"
        "请输入新标尺名称。").arg(rail->name(), ruler->name()), QLineEdit::Normal,
        ruler->name() + tr("_副本"), &ok);
    if (!ok)return;
    if (name.isEmpty() || rail->rulerNameExisted(name)) {
        QMessageBox::warning(mw, tr("错误"), tr("标尺名称为空或已存在！"));
        return;
    }
    std::shared_ptr<Railway> data = ruler->clone();
    data->getRuler(0)->setName(name);

    auto* stk = mw->getUndoStack();
    stk->beginMacro(tr("创建标尺副本: %1").arg(name));
    stk->push(new qecmd::AddNewRuler(name, rail, mw->getRailContext()));
    auto newruler = rail->rulers().last();
    stk->push(new qecmd::UpdateRuler(newruler, data, this));
    stk->endMacro();
}

void RulerContext::mergeWith(std::shared_ptr<Ruler> ruler)
{
    auto rail = ruler->railway();
    // 现在：现场构造一个Dialog..
    auto* dialog = new QDialog(mw);
    dialog->setWindowTitle(tr("合并标尺"));
    auto* vlay = new QVBoxLayout(dialog);
    auto* lab = new QLabel(tr("从本线路的下列标尺合并数据到当前标尺[%1]").arg(ruler->name()));
    vlay->addWidget(lab);
    auto* cb = new RulerCombo(rail);
    vlay->addWidget(cb);
    auto* ck = new QCheckBox(tr("对重复的数据，覆盖当前标尺既有数据"));
    vlay->addWidget(ck);
    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    connect(box, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    vlay->addWidget(box);
    auto t = dialog->exec();
    ////
    auto ref = cb->ruler();
    if (!t || !ref) {
        qDebug() << "merge ruler cancled" << Qt::endl;
        return;
    }
    else if (ref == ruler) {
        QMessageBox::warning(mw, tr("错误"), tr("源数据标尺和当前标尺重复，无需合并。"));
        return;
    }
    auto data_r = ruler->clone();   // 更新在这个对象里面
    auto data = data_r->getRuler(0);
    
    data->mergeWith(*ref, ck->isChecked());
    mw->getUndoStack()->push(new qecmd::UpdateRuler(ruler, data_r, this));

    // 清理掉dialog
    dialog->setParent(nullptr);
    dialog->deleteLater();
}

void RulerContext::actDulplicateRuler()
{
    dulplicateRuler(this->ruler);
}

void RulerContext::mergeCurrent()
{
    mergeWith(this->ruler);
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
    ruler->railway()->undoRemoveRuler(ruler, data);
    if (isOrd) {
        ruler->railway()->setOrdinate(ruler);
    }
    cont->undoRemoveRuler(ruler, isOrd);
}

void qecmd::RemoveRuler::redo()
{
    ruler->railway()->removeRuler(ruler);
    cont->commitRemoveRuler(ruler, isOrd);
}

#endif
