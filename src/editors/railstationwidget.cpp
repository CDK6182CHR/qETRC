#include "railstationwidget.h"
#include "util/buttongroup.hpp"
#include "model/delegate/combodelegate.h"
#include "model/delegate/generaldoublespindelegate.h"

#include "model/rail/railstationmodel.h"
#include "util/qecontrolledtable.h"
#include "data/common/qesystem.h"
#include "data/rail/railway.h"
#include "data/rail/railcategory.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <QEvent>
#include <QPlainTextEdit>
#include <QLabel>
#include <QScroller>
#include <QAction>


RailStationWidget::RailStationWidget(RailCategory& cat_, bool inplace, QWidget* parent) :
    QWidget(parent), model(new RailStationModel(inplace, this)), commitInPlace(inplace), cat(cat_)
{
	//暂定Model的Parent就是自己！
	//setFocusPolicy(Qt::ClickFocus);
	initUI();
	connect(edName, &QLineEdit::textChanged, this, &RailStationWidget::markChanged);
	connect(model, &RailStationModel::dataSubmitted, this, &RailStationWidget::markChanged);
}

void RailStationWidget::setRailway(std::shared_ptr<Railway> rail)
{
	railway = rail;
	model->setRailway(rail);
	if (railway) {
		edName->setText(railway->name());
	}
	else {
		edName->clear();
	}
	_changed = false;
}

void RailStationWidget::refreshBasicData()
{
	if (railway)
		edName->setText(railway->name());
	else edName->clear();
}

void RailStationWidget::refreshData()
{
	if (!railway)return;
	model->refreshData();
	edName->setText(railway->name());
	_changed = false;
}

bool RailStationWidget::applyChange()
{
	actApply();
    return !_changed;
}

void RailStationWidget::setReadOnly()
{
    edName->setReadOnly(true);
    edName->setAttribute(Qt::WA_InputMethodEnabled,false);
    ctable->table()->setEditTriggers(QTableView::NoEditTriggers);
}

bool RailStationWidget::event(QEvent* e)
{
	if (e->type() == QEvent::WindowActivate) {
		if (railway)
			emit focusInRailway(railway);
		return true;
	}
	return QWidget::event(e);
}

void RailStationWidget::initUI()
{
	auto* vlay = new QVBoxLayout;

	auto* form = new QFormLayout;
	edName = new QLineEdit;
	form->addRow(tr("线名"), edName);
	vlay->addLayout(form);

	ctable = new QEControlledTable;
	ctable->table()->verticalHeader()->setDefaultSectionSize(25);
	ctable->table()->setModel(model);
	auto* dele = new ComboDelegate({ tr("不通过"),tr("下行"),tr("上行"),tr("上下行") }, this);
	ctable->table()->setItemDelegateForColumn(RailStationModel::ColDir, dele);
	ctable->table()->setEditTriggers(QTableView::AllEditTriggers);
	ctable->table()->setItemDelegateForColumn(RailStationModel::ColMile,
		new GeneralDoubleSpinDelegate(this));
    ctable->table()->verticalHeader()->setDefaultSectionSize(
                SystemJson::instance.table_row_height);
    QScroller::grabGesture(ctable->table(),QScroller::TouchGesture);

	ctable->table()->setContextMenuPolicy(Qt::ActionsContextMenu);

	// context menu
	auto* act = new QAction(tr("批量勾选"), ctable);
	connect(act, &QAction::triggered, ctable, &QEControlledTable::checkSelection);
	ctable->table()->addAction(act);

	act = new QAction(tr("批量取消勾选"), ctable);
	connect(act, &QAction::triggered, ctable, &QEControlledTable::uncheckSelection);
	ctable->table()->addAction(act);

	act = new QAction(tr("批量切换选择"), ctable);
	connect(act, &QAction::triggered, ctable, &QEControlledTable::toggleSelection);
	ctable->table()->addAction(act);

	int c = 0;
	for (int w : {120, 80, 80, 50, 40, 60, 40, 40, 40}) {
		ctable->table()->setColumnWidth(c++, w);
	}

	vlay->addWidget(ctable);

	auto* g = new ButtonGroup<3>({ "确定","还原","备注" });
	g->setMinimumWidth(50);
	g->connectAll(SIGNAL(clicked()), this, { SLOT(actApply()),SLOT(actCancel()),
		SLOT(actNote()) });
	vlay->addLayout(g);
	
	setLayout(vlay);
}

void RailStationWidget::actCancel()
{
	refreshData();
}

void RailStationWidget::markChanged()
{
    _changed = true;
}

void RailStationWidget::actNote()
{
    auto* dia=new RailNoteDialog(railway,commitInPlace,this);
    connect(dia,&RailNoteDialog::railNoteChanged,
            this,&RailStationWidget::railNoteChanged);
    dia->open();
}

void RailStationWidget::actApply()
{
	if (!railway) {
		// 这种情况只可能在线路数据库中发生。
		// 如果出现，什么都不做，RailDBWindow会判定Index的有效性
		emit invalidApplyRequest();
		return;
	}
	//先讨论线名的修改
	const QString& name = edName->text();
	if (name != railway->name()) {
		if (!cat.railNameIsValidRec(name, railway)) {
			QMessageBox::warning(this, tr("错误"), tr("线路名称不能为空或与其他线名冲突，请重新设置。"));
			return;
		}
		if (commitInPlace) {
			railway->setName(name);
		}
		else {
			emit railNameChanged(railway, name);
		}
	}
	model->actApply();
	_changed = false;
}

RailNoteDialog::RailNoteDialog(std::shared_ptr<Railway> railway_, bool inplace, QWidget* parent):
	QDialog(parent),railway(railway_),commitInPlace(inplace)
{
	resize(400, 400);
	setWindowTitle(tr("备注 - %1").arg(railway->name()));
	setAttribute(Qt::WA_DeleteOnClose);
    initUI();
    refreshData();
}

void RailNoteDialog::refreshData()
{
    edAuthor->setText(railway->notes().author);
    edVersion->setText(railway->notes().version);
    edNote->setPlainText(railway->notes().note);
}

void RailNoteDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;
    edAuthor=new QLineEdit;
    flay->addRow(tr("作者"),edAuthor);
    edVersion=new QLineEdit;
    flay->addRow(tr("版本"),edVersion);
    edNote=new QPlainTextEdit;
    vlay->addLayout(flay);
    vlay->addWidget(new QLabel(tr("其他说明：")));
    vlay->addWidget(edNote);

    auto* g=new ButtonGroup<2>({"确定","取消"});
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(close())});
    vlay->addLayout(g);
}

void RailNoteDialog::actApply()
{
    RailInfoNote data;
    data.author=edAuthor->text();
    data.version=edVersion->text();
    data.note=edNote->toPlainText();
    if (data != railway->notes()){
        emit railNoteChanged(railway,data);
    }
	done(QDialog::Accepted);
}
