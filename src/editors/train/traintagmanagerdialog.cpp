#include "traintagmanagerdialog.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>

#include "data/train/traintagmanager.h"
#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"
#include "edittraintagdialog.h"
#include "tagtrainsdialog.h"

TrainTagManagerModel::TrainTagManagerModel(TrainTagManager& manager, QObject* parent):
	QAbstractTableModel(parent), m_manager(manager)
{
	refreshData();
}

int TrainTagManagerModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return m_tagNames.size();
}

int TrainTagManagerModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return ColMAX;
}

QVariant TrainTagManagerModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return {};

	const auto& name = m_tagNames.at(index.row());
	auto tag = m_manager.find(name);

	if (role == Qt::EditRole || role == Qt::DisplayRole) {
		switch (index.column()) {
		case ColName: return name;
		case ColNote: 
			if (tag) {
				return tag->note();
			}
			else {
				qCritical() << "TrainTagManagerModel::data: INTERNAL ERROR: the tag named " << name << " does not exist";
				return tr("!!ERROR: tag does not exist!!");
			}
		}
	}
	return {};
}

QVariant TrainTagManagerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) {
			switch (section) {
			case ColName: return tr("标签名"); break;
			case ColNote: return tr("备注"); break;
			}
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

std::shared_ptr<TrainTag> TrainTagManagerModel::tagForRow(int row)
{
	if (row < 0 || row >= m_tagNames.size())
		return nullptr;
	const auto& name = m_tagNames.at(row);
	auto tag = m_manager.find(name);
	if (!tag) {
		qCritical() << "TrainTagManagerModel: INTERNAL ERROR: tag not found for name " << name;
	}
	return tag;
}

void TrainTagManagerModel::refreshData()
{
	beginResetModel();
	m_tagNames.clear();
	for (const auto& p : m_manager.tags()) {
		m_tagNames.push_back(p.second->name());
	}
	endResetModel();
}

TrainTagManagerDialog::TrainTagManagerDialog(DiagramOptions& options, TrainCollection& coll, TrainTagManager& manager, QWidget* parent):
	QDialog(parent), m_options(options), m_coll(coll), m_manager(manager)
{
	setAttribute(Qt::WA_DeleteOnClose);
	initUI();
}

void TrainTagManagerDialog::initUI()
{
	resize(600, 600);
	setWindowTitle(tr("列车标签管理"));
	
	auto* vlay = new QVBoxLayout(this);
	auto* lab = new QLabel(tr("以下是本运行图文件中的所有列车标签。"
		"此处可新增标签、修改标签名或备注、删除标签。请注意删除标签将同时从所有列车移除该标签。"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	m_model = new TrainTagManagerModel(m_manager, this);
	m_table = new QTableView;
	m_table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
	m_table->setEditTriggers(QTableView::NoEditTriggers);
	m_table->setSelectionBehavior(QTableView::SelectRows);
	m_table->setModel(m_model);

	{
		int c = 0;
		for (int w : {100, 300}) {
			m_table->setColumnWidth(c++, w);
		}
	}
	vlay->addWidget(m_table);

	auto* g = new ButtonGroup<5>({ "添加", "删除", "编辑", "查看列车", "关闭"});
	g->connectAll(SIGNAL(clicked()), this,
		{ SLOT(actAdd()), SLOT(actRemove()), SLOT(actEdit()), SLOT(actViewTrains()), SLOT(close())});
	vlay->addLayout(g);
}

std::shared_ptr<TrainTag> TrainTagManagerDialog::currentTag()
{
	const auto& idx = m_table->currentIndex();
	if (idx.isValid()) {
		return m_model->tagForRow(idx.row());
	}
	return {};
}

void TrainTagManagerDialog::refreshData()
{
	m_model->refreshData();
}

void TrainTagManagerDialog::actAdd()
{
	auto* dlg = new EditTrainTagDialog(m_manager, this);
	connect(dlg, &EditTrainTagDialog::tagAdded, this, &TrainTagManagerDialog::tagAdded);
	dlg->open();
}

void TrainTagManagerDialog::actRemove()
{
	auto tag = currentTag();
	if (!tag)
		return;
	if (QMessageBox::question(this, tr("删除标签"), 
		tr("将此标签从管理器删除，同时从所有列车中删除此标签，是否确认？")) != QMessageBox::Yes)
		return;

	emit removeTag(tag);
}

void TrainTagManagerDialog::actEdit()
{
	auto tag = currentTag();
	if (!tag)
		return;
	auto* dlg = new EditTrainTagDialog(tag, m_manager, this);
	connect(dlg, &EditTrainTagDialog::tagNameChanged, this, &TrainTagManagerDialog::tagNameChanged);
	connect(dlg, &EditTrainTagDialog::tagNoteChanged, this, &TrainTagManagerDialog::tagNoteChanged);
	dlg->open();
}


void TrainTagManagerDialog::actViewTrains()
{
	auto tag = currentTag();
	if (!tag) return;
	auto* dlg = new TagTrainsDialog(m_options, m_coll, tag, this);
	connect(dlg, &TagTrainsDialog::removeTrains,
		this, &TrainTagManagerDialog::removeTagFromTrains);
	dlg->open();
}
