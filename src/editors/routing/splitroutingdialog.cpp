#include "splitroutingdialog.h"

#include <set>

#include <QVBoxLayout>
#include <QFormLayout>
#include <QTableView>
#include <QLineEdit>
#include <QHeaderView>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "data/train/routing.h"
#include "data/train/train.h"
#include "util/utilfunc.h"
#include "data/common/qesystem.h"
#include "data/train/traincollection.h"

SplitRoutingModel::SplitRoutingModel(std::shared_ptr<Routing> routing, QObject* parent):
	QStandardItemModel(parent), m_routing(routing)
{
	setHorizontalHeaderLabels({
		tr("车次"),tr("虚拟"),tr("始发"),tr("终到"),
		tr("连线"),tr("拆分"),tr("新交路名")
		});
}

void SplitRoutingModel::refreshData()
{
	using SI = QStandardItem;
	setRowCount(m_routing->count());
	int i = 0;
	for (auto itr = m_routing->order().begin(); itr != m_routing->order().end(); ++itr, ++i) {
		setItem(i, ColTrainName, qeutil::makeReadOnlyItem(itr->name()));
		auto* it = qeutil::makeCheckItem();
		it->setEditable(false);
		it->setCheckState(qeutil::boolToCheckState(itr->isVirtual()));
		it->setCheckable(false);
		setItem(i, ColVirtual, it);

		if (itr->isVirtual()) {
			setItem(i, ColStarting, qeutil::makeReadOnlyItem(itr->virtualStarting()));
			setItem(i, ColTerminal, qeutil::makeReadOnlyItem(itr->virtualTerminal()));
		}
		else {
			setItem(i, ColStarting, qeutil::makeReadOnlyItem(itr->train()->starting().toSingleLiteral()));
			setItem(i, ColTerminal, qeutil::makeReadOnlyItem(itr->train()->terminal().toSingleLiteral()));
		}

		it = qeutil::makeCheckItem();
		it->setCheckable(false);
		it->setEditable(false);
		it->setCheckState(qeutil::boolToCheckState(itr->link()));
		setItem(i, ColLink, it);

		it = qeutil::makeCheckItem();
		it->setEditable(false);
		it->setCheckState(Qt::Unchecked);
		setItem(i, ColSplit, it);

		setItem(i, ColNewName, new SI(""));
	}
}

std::vector<SplitRoutingModel::Result> SplitRoutingModel::appliedData() const
{
	std::vector<Result> res{};
	for (int row = 0; row < rowCount(); row++) {
		if (item(row, ColSplit)->checkState() == Qt::Checked) {
			res.emplace_back(Result{ .index = row, .new_name = item(row,ColNewName)->text() });
		}
	}
	return res;
}

SplitRoutingDialog::SplitRoutingDialog(TrainCollection& coll, std::shared_ptr<Routing> routing, QWidget* parent):
	QDialog(parent),m_coll(coll), m_routing(routing),
	m_model(new SplitRoutingModel(routing, this))
{
	initUI();
	refreshData();
	setAttribute(Qt::WA_DeleteOnClose, true);
}

void SplitRoutingDialog::accept()
{
	auto data = m_model->appliedData();
	if (data.empty()) {
		QMessageBox::information(this, tr("提示"), tr("没有拆分操作"));
		return;
	}

	std::set<QString> new_names{};
	QString report;
	std::vector<SplitRoutingData> res;
	std::list<RoutingNode>::iterator itr{};
	int last_index = 0;
	QString last_name;

	auto add_routing_func = [&](const QString& name, int new_idx, int last_idx, auto itr) {
		report.append(tr("新交路[%1] 首车次[%2] 共%3车次\n").arg(name, itr->name()).arg(new_idx - last_index));
		res.emplace_back(SplitRoutingData{ .name = name, .idx_first = last_idx, .idx_last = new_idx });
	};

	for (const auto& d : data) {
		if (d.new_name.isEmpty()) {
			QMessageBox::warning(this, tr("错误"), tr("第%1行的新交路没有指定交路名").arg(d.index + 1));
			return;
		}
		if (new_names.contains(d.new_name) || m_coll.routingNameExisted(d.new_name)) {
			QMessageBox::warning(this, tr("错误"), tr("第%1行的新交路名称[%2]重复").arg(d.index + 1).arg(d.new_name));
			return;
		}

		// TODO: convert data; output abstract.
		if (last_index == 0) {
			// This is the first node
			report.append(tr("原交路[%1] 剩余%2车次\n").arg(m_routing->name()).arg(d.index));
			auto itr2 = m_routing->order().begin();
			std::advance(itr2, d.index);
			itr = itr2;
			last_index = d.index;
			last_name = d.new_name;
		}
		else {
			auto itr2 = itr;
			std::advance(itr2, d.index - last_index);
			add_routing_func(last_name, d.index, last_index, itr);

			itr = itr2;
			last_index = d.index;
			last_name = d.new_name;
		}
	}

	if (itr != m_routing->order().end()) {
		add_routing_func(last_name, m_routing->count(), last_index, itr);
	}

	auto flag = QMessageBox::question(this, tr("交路拆分"), tr("交路拆分概要如下，是否继续？\n") + report);
	if (flag != QMessageBox::Yes)
		return;

	emit splitApplied(m_routing, res);
	QDialog::accept();
}

void SplitRoutingDialog::initUI()
{
	auto* vlay = new QVBoxLayout(this);
	auto* form = new QFormLayout;

	setWindowTitle(tr("交路拆分 - %1").arg(m_routing->name()));
	resize(800, 600);

	auto* lab = new QLabel(tr("本功能支持将一个交路拆分成若干交路。请在下表中，要拆分交路的起始车次（包含该车次）行的“拆分”列"
		"勾选，系统将从所选行开始，至下一所选行之前，或当前交路的最后一个车次，构建新交路。"
		"请在所勾选行的“新交路名”列填写新交路的名称。"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	m_edName = new QLineEdit;
	form->addRow(tr("交路名"), m_edName);
	vlay->addLayout(form);

	m_table = new QTableView;
	m_table->setModel(m_model);
	m_table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
	m_table->setEditTriggers(QTableView::AllEditTriggers);
	vlay->addWidget(m_table);

	{
		int c = 0;
		for (int w : {120, 40, 100, 100, 40, 40, 100}) {
			m_table->setColumnWidth(c++, w);
		}
	}

	auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
		Qt::Horizontal);
	vlay->addWidget(box);

	connect(box, &QDialogButtonBox::accepted, this, &SplitRoutingDialog::accept);
	connect(box, &QDialogButtonBox::rejected, this, &SplitRoutingDialog::close);
}

void SplitRoutingDialog::refreshData()
{
	m_edName->setText(m_routing->name());
	m_model->refreshData();
}
