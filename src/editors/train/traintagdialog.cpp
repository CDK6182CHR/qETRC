#include "traintagdialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>

#include "data/train/traintag.h"
#include "data/train/train.h"
#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"


TrainTagModel::TrainTagModel(QObject* parent):
	QStandardItemModel(parent)
{
	setColumnCount(ColMAX);
	setHorizontalHeaderLabels({ tr("标签名"), tr("备注") });
}

void TrainTagModel::refreshData(const std::vector<std::shared_ptr<TrainTag>>& tags)
{
	using SI = QStandardItem;
	setRowCount(tags.size());

	for (int r = 0; r < tags.size(); r++) {
		auto tag = tags.at(r);

		setItem(r, ColName, new SI(tag->name()));

		auto* itemNote = new SI(tag->note());
		itemNote->setEditable(false);
		itemNote->setToolTip(tag->note());
		setItem(r, ColNote, itemNote);
	}
}

TrainTagDialog::TrainTagDialog(TrainTagManager& tagman, std::shared_ptr<Train> train, QWidget* parent):
	QDialog(parent), m_tagMan(tagman), m_train(train)
{
	setAttribute(Qt::WA_DeleteOnClose, true);
	initUI();
	refreshData();
}

void TrainTagDialog::initUI()
{
	setWindowTitle(tr("列车标签 - %1").arg(m_train->trainName().full()));
	resize(600, 400);
	auto* vlay = new QVBoxLayout(this);

	auto* lab = new QLabel(tr("以下是本次列车已有的列车标签。" 
		"本运行图文件内所有同名列车标签被视为同一个列车标签。添加不存在的标签将自动创建该标签。\n"
		"备注栏只作为参考，不可编辑。如需编辑，请移步列车标签管理对话框。\n"
		"请注意，所有添加、删除操作均立即生效。"
	));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	auto* form = new QFormLayout;
	auto* hlay = new QHBoxLayout;
	
	m_edNewTag = new QLineEdit;
	hlay->addWidget(m_edNewTag);
	auto* btnAdd = new QPushButton(tr("添加"));
	connect(btnAdd, &QPushButton::clicked, this, &TrainTagDialog::actAddNewTag);
	hlay->addWidget(btnAdd);
	form->addRow(tr("添加新标签"), hlay);
	vlay->addLayout(form);

	m_model = new TrainTagModel(this);
	m_table = new QTableView;
	m_table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->setSelectionMode(QAbstractItemView::SingleSelection);
	m_table->setModel(m_model);
	vlay->addWidget(m_table);

	{
		int c = 0;
		for (int w : {120, 300}) {
			m_table->setColumnWidth(c++, w);
		}
	}

	auto* g = new ButtonGroup<3>({ "删除标签", "刷新数据", "关闭对话框" });
	vlay->addLayout(g);
	g->connectAll(SIGNAL(clicked()), this, { SLOT(actDeleteTag()), SLOT(refreshData()), SLOT(close()) });
}

void TrainTagDialog::actAddNewTag()
{
	const QString& name = m_edNewTag->text().trimmed();
	if (name.isEmpty()) {
		QMessageBox::warning(this, tr("无效的标签名"), tr("标签名不能为空，请重新输入。"));
		return;
	}
	m_edNewTag->clear();
	emit addTrainTag(m_train, name);
}

void TrainTagDialog::actDeleteTag()
{
	auto sel = m_table->currentIndex();
	if (!sel.isValid()) {
		return;
	}

	emit removeTrainTag(m_train, sel.row());
}

void TrainTagDialog::refreshData()
{
	m_model->refreshData(m_train->tags());
}
