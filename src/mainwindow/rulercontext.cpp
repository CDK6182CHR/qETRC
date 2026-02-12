#ifndef QETRC_MOBILE_2
#include "data/rail/rulernode.h"
#include "data/rail/ruler.h"
#include "rulercontext.h"
#include "mainwindow.h"
#include "railcontext.h"
#include "defines/icon_specs.h"
#include "wizards/greedypaint/greedypaintwizard.h"
#include "util/railrulercombo.h"

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
#include <QFileDialog>

RulerContext::RulerContext(Diagram& diagram_, SARibbonContextCategory *context, MainWindow *mw_):
    QObject(mw_),diagram(diagram_), cont(context),mw(mw_)
{
    initUI();
}

void RulerContext::setRuler(std::shared_ptr<Ruler> r)
{
    this->ruler=r;
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
    auto* panel = page->addPanel(tr("当前标尺"));

    auto* w = new QWidget;
    auto* hlay = new QHBoxLayout(w);
    auto* ed = new QLineEdit;
    ed->setFocusPolicy(Qt::NoFocus);
    ed->setAlignment(Qt::AlignCenter);
    //ed->setFixedWidth(150);
    ed->setToolTip(tr("当前标尺名称\n不可编辑；如需编辑请至编辑面板操作"));
    edRulerName = ed;
    hlay->addWidget(ed);
    auto* btn = new SARibbonToolButton();
    btn->setIcon(QEICN_change_ruler);
    btn->setButtonType(SARibbonToolButton::SmallButton);
    connect(btn, &QToolButton::clicked, this, &RulerContext::actChangeRuler);
    hlay->addWidget(btn);
    hlay->setContentsMargins(0, 0, 0, 0);

    w->setObjectName(tr("标尺名称-只读"));
    w->setWindowTitle(tr("标尺名称"));
    panel->addWidget(w, SARibbonPanelItem::Medium);

    ed = new QLineEdit;
    ed->setFocusPolicy(Qt::NoFocus);
    ed->setAlignment(Qt::AlignCenter);
    ed->setToolTip(tr("当前标尺所属线路"));
    edRailName = ed;
    ed->setFixedWidth(150);
    ed->setObjectName(tr("标尺线路名称"));
    ed->setWindowTitle(tr("标尺线路名称"));
    panel->addWidget(ed, SARibbonPanelItem::Medium);

    panel = page->addPanel(tr("编辑"));
    auto* act = mw->makeAction(QEICN_edit_ruler, tr("编辑"), tr("编辑标尺"));
    act->setToolTip(tr("标尺编辑面板\n显示当前标尺的编辑面板，编辑标尺名称和数据。"));
    connect(act, SIGNAL(triggered()), this, SLOT(actShowEditWidget()));
    panel->addLargeAction(act);

    act = mw->makeAction(QEICN_ruler_from_train, tr("从车次提取"));
    act->setToolTip(tr("从单车次提取标尺\n"
        "从单个车次在本线的运行情况提取标尺数据，并覆盖到本标尺中。"));
    connect(act, SIGNAL(triggered()), this, SLOT(actReadFromSingleTrain()));
    panel->addMediumAction(act);

    act = mw->makeAction(QEICN_ruler_from_speed, tr("从速度计算"));
    act->setToolTip(tr("从运行速度计算\n"
        "从通通运行速度（不含起停附加时分）计算近似标尺，并覆盖到本标尺中。"));
    connect(act, &QAction::triggered, this, &RulerContext::actFromSpeed);
    panel->addMediumAction(act);

    act = mw->makeAction(QEICN_merge_ruler, tr("合并标尺"));
    act->setToolTip(tr("合并标尺\n与本线路的其他标尺合并。"));
    connect(act, &QAction::triggered, this, &RulerContext::mergeCurrent);
    panel->addMediumAction(act);

    panel = page->addPanel(tr("导入导出"));
    act = mw->makeAction(QEICN_import_ruler_csv, tr("导入"), tr("导入标尺"));
    act->setToolTip(tr("导入标尺 (csv)\n"
        "从逗号分隔 (csv) 格式的数据文件中读取标尺数据，并覆盖到当前标尺中"));
    panel->addLargeAction(act);
    connect(act, &QAction::triggered, this, &RulerContext::actImportCsv);

    act = mw->makeAction(QEICN_export_ruler_csv, tr("导出"), tr("导出标尺"));
    act->setToolTip(tr("导出标尺 (csv)\n将当前标尺导出为逗号分隔 (csv) 格式的数据文件"));
    panel->addLargeAction(act);
    connect(act, &QAction::triggered, this, &RulerContext::actExportCsv);

    panel = page->addPanel(tr("设置"));
    act = mw->makeAction(QEICN_ordinate_ruler, tr("排图标尺"));
    act->setToolTip(tr("设为排图标尺\n将当前标尺设置为本线的排图标尺，即作为纵坐标标度"));
    panel->addLargeAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(actSetAsOrdinate()));

    act = mw->makeAction(QEICN_copy_ruler, tr("副本"), tr("标尺副本"));
    act->setToolTip(tr("创建标尺副本\n使用输入的标尺名称，在本线路下创建当前标尺副本"));
    panel->addLargeAction(act);
    connect(act, &QAction::triggered, this, &RulerContext::actDulplicateRuler);

    act = mw->makeAction(QEICN_del_ruler, tr("删除标尺"));
    act->setToolTip(tr("删除标尺\n删除当前标尺。如果当前标尺同时是排图标尺，"
        "则同时将本线设置为按里程排图。"));
    panel->addLargeAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(actRemoveRuler()));

    act = mw->makeAction(QEICN_close_ruler_context, tr("关闭面板"), tr("关闭标尺面板"));
    act->setToolTip(tr("关闭面板\n关闭当前的标尺上下文工具栏页面"));
    panel->addLargeAction(act);
    connect(act, &QAction::triggered, this, &RulerContext::focusOutRuler);
    
}

void RulerContext::actChangeRulerData(std::shared_ptr<Ruler> r, std::shared_ptr<Railway> nr)
{
    mw->getUndoStack()->push(new qecmd::UpdateRuler(r, nr, this));
}

void RulerContext::commitRulerChange(std::shared_ptr<Ruler> r)
{
    mw->getRailContext()->refreshRulerTable(r);
    if (r->isOrdinateRuler()) {
        emit ordinateRulerModified(r->railway());
    }
}

void RulerContext::actChangeRulerName(std::shared_ptr<Ruler> r, const QString& name)
{
    mw->getUndoStack()->push(new qecmd::ChangeRulerName(r, name, this));
}

void RulerContext::commitChangeRulerName(std::shared_ptr<Ruler> r)
{
    if (r == this->ruler) {
        refreshData();
    }
    mw->getRailContext()->onRulerNameChanged(r);
    emit rulerNameChanged(r);
}

void RulerContext::commitRemoveRuler(std::shared_ptr<Ruler> r, bool isord)
{
    // 实施删除标尺：从线路中移除数据；退出当前面板；如果是ord则撤销ord
    auto rail = r->railway();

    mw->getRailContext()->removeRulerAt(*(r->railway()), r->index(), isord);
    mw->getRailContext()->removeRulerWidget(r);

    if (isord) {
        //重新排图
        mw->updateRailwayDiagrams(rail);
    }

    if(r==this->ruler)
        emit focusOutRuler();

    // 2024.04.07: for greedy painter
    if (mw->greedyWidget) {
        mw->greedyWidget->onRulerRemoved(r, rail);
    }
}

void RulerContext::undoRemoveRuler(std::shared_ptr<Ruler> r, bool isord)
{
    if (isord) {
        mw->updateRailwayDiagrams(r->railway());
    }
    mw->getRailContext()->insertRulerAt(*(r->railway()), r, isord);
}

void RulerContext::actRemoveRulerNavi(std::shared_ptr<Ruler> ruler1)
{
    auto r = ruler1->clone();
    mw->getUndoStack()->push(new qecmd::RemoveRuler(ruler1, r, ruler1->isOrdinateRuler(),
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
    auto* dialog = new RulerFromTrainDialog(diagram.options(), diagram.trainCollection(), ruler, mw);
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

void RulerContext::dulplicateRuler(std::shared_ptr<Ruler> r)
{
    if (!r)
        return;
    auto rail = r->railway();

    bool ok;
    auto name = QInputDialog::getText(mw, tr("创建标尺副本"), tr("现在创建基线[%1]中标尺[%2]的副本。"
        "请输入新标尺名称。").arg(rail->name(), r->name()), QLineEdit::Normal,
        r->name() + tr("_副本"), &ok);
    if (!ok)return;
    if (name.isEmpty() || rail->rulerNameExisted(name)) {
        QMessageBox::warning(mw, tr("错误"), tr("标尺名称为空或已存在！"));
        return;
    }
    std::shared_ptr<Railway> data = r->clone();
    data->getRuler(0)->setName(name);

    auto* stk = mw->getUndoStack();
    stk->beginMacro(tr("创建标尺副本: %1").arg(name));
    stk->push(new qecmd::AddNewRuler(name, rail, mw->getRailContext()));
    auto newruler = rail->rulers().last();
    stk->push(new qecmd::UpdateRuler(newruler, data, this));
    stk->endMacro();
}

void RulerContext::mergeWith(std::shared_ptr<Ruler> r)
{
    auto rail = r->railway();
    // 现在：现场构造一个Dialog..
    auto* dialog = new QDialog(mw);
    dialog->setWindowTitle(tr("合并标尺"));
    auto* vlay = new QVBoxLayout(dialog);
    auto* lab = new QLabel(tr("从本线路的下列标尺合并数据到当前标尺[%1]").arg(r->name()));
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
    else if (ref == r) {
        QMessageBox::warning(mw, tr("错误"), tr("源数据标尺和当前标尺重复，无需合并。"));
        return;
    }
    auto data_r = r->clone();   // 更新在这个对象里面
    auto data = data_r->getRuler(0);
    
    data->mergeWith(*ref, ck->isChecked());
    mw->getUndoStack()->push(new qecmd::UpdateRuler(r, data_r, this));

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

void RulerContext::actExportCsv()
{
    if (!ruler) return;
    QString init_filename = QString("%1_%2").arg(ruler->railway()->name(), ruler->name());
    init_filename.replace("*", "-");   // * cannot appeared in filenames
    const QString& filename = QFileDialog::getSaveFileName(mw, tr("导出标尺"),
        init_filename,
        tr("逗号分隔文件 (*.csv)\n所有文件 (*)"));
    if (filename.isEmpty())
        return;
    bool flag = ruler->toCsv(filename);
    if (flag)
        mw->showStatus(tr("导出标尺成功"));
    else
        QMessageBox::warning(mw, tr("错误"), tr("导出标尺失败"));
}

void RulerContext::actImportCsv()
{
    if (!ruler) return;
    auto res = QMessageBox::question(mw, tr("导入标尺"),
        tr("从CSV文件导入标尺，其中每一行应有5列：发站、到站、通通时分、起步附加、停车附加，"
            "所有时间以秒为单位。使用导出标尺功能可获得示例文件。\n"
            "与当前线路匹配的区间的标尺将被导入并覆盖原数据，不匹配的将被忽略。是否继续？"));
    if (res != QMessageBox::Yes)
        return;

    const QString& filename = QFileDialog::getOpenFileName(mw, tr("导入标尺"),
        {},
        tr("逗号分隔文件 (*.csv)\n所有文件 (*)"));
    if (filename.isEmpty())
        return;

    auto r = ruler->clone();
    int val = r->rulers().front()->fromCsv(filename);
    if (val) {
        mw->getUndoStack()->push(new qecmd::UpdateRuler(ruler, r, this));
    }
    QMessageBox::information(mw, tr("提示"),
        tr("已导入%1条数据").arg(val));
}

void RulerContext::actChangeRuler()
{
    auto ruler = RailRulerCombo::dialogGetRuler(diagram.railCategory(), mw, tr("选择标尺"),
        tr("请选择要切换到的标尺"));
    if (ruler) {
        mw->focusInRuler(ruler);
    }
}

void RulerContext::actShowEditWidget()
{
    mw->getRailContext()->openRulerWidget(ruler);
}

qecmd::UpdateRuler::UpdateRuler(std::shared_ptr<Ruler> ruler_, std::shared_ptr<Railway> nr_,
                               RulerContext *context, QUndoCommand *parent):
    QUndoCommand(QObject::tr("更新标尺数据: ")+ruler_->name(),parent),
    ruler(ruler_),nr(nr_), cont(context)
{

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

qecmd::RemoveRuler::RemoveRuler(std::shared_ptr<Ruler> ruler_, std::shared_ptr<Railway> data_,
                      bool ordinate, RulerContext *context, QUndoCommand *parent):
    QUndoCommand(QObject::tr("删除标尺: ")+ruler_->name(),parent),
    ruler(ruler_),data(data_),isOrd(ordinate),cont(context){}

void qecmd::RemoveRuler::undo()
{
    ruler->railway()->undoRemoveRuler(ruler, data);
    if (isOrd) {
        ruler->railway()->setOrdinate(ruler);
        ruler->railway()->calStationYCoeff();   // 2024.04.10 add
    }
    cont->undoRemoveRuler(ruler, isOrd);
}

void qecmd::RemoveRuler::redo()
{
    ruler->railway()->removeRuler(ruler);
    cont->commitRemoveRuler(ruler, isOrd);
}

#endif
