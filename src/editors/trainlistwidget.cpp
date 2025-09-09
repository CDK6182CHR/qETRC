#include "trainlistwidget.h"

#ifndef QETRC_MOBILE_2

#include <ranges>

#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

#include "util/buttongroup.hpp"
#include "data/train/train.h"
#include "data/train/traincollection.h"
#include "model/train/trainlistmodel.h"
#include "mainwindow/traincontext.h"
#include "data/diagram/trainline.h"
#include "editors/train/trainpenwidget.h"
#include "editors/train/trainfilterselector.h"

TrainListWidget::TrainListWidget(const DiagramOptions& ops, TrainCollection& coll_, QUndoStack* undo, QWidget* parent):
    QWidget(parent), _ops(ops), coll(coll_),_undo(undo),
    table(new QTableView),editSearch(new QLineEdit), model(new TrainListModel(_ops, coll_, undo, this))
{
	initUI();
}

void TrainListWidget::refreshData()
{
	model->beginResetModel();
	model->endResetModel();
	table->resizeColumnsToContents();
}

void TrainListWidget::initUI()
{
	auto* vlay = new QVBoxLayout;

	filter = new TrainFilterSelector(coll);
	vlay->addWidget(filter);
	connect(filter, &TrainFilterSelector::filterChanged,
		this, &TrainListWidget::onFilterChanged);

	auto* hlay = new QHBoxLayout;
	hlay->addWidget(editSearch);   //搜索行为改为filt
	connect(editSearch, SIGNAL(editingFinished()), this, SLOT(searchTrain()));
	auto* btn = new QPushButton(QObject::tr("搜索"));
	btn->setMinimumWidth(50);
	connect(btn, SIGNAL(clicked()), this, SLOT(searchTrain()));
	hlay->addWidget(btn);
	btn = new QPushButton(tr("清空搜索"));
	btn->setMinimumWidth(70);
	connect(btn, SIGNAL(clicked()), this, SLOT(clearFilter()));
	hlay->addWidget(btn);
	vlay->addLayout(hlay);

	table->verticalHeader()->setDefaultSectionSize(20);
	table->setModel(model);
	table->setSelectionBehavior(QTableView::SelectRows);
	table->horizontalHeader()->setSortIndicatorShown(true);
	connect(table->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
		table, SLOT(sortByColumn(int, Qt::SortOrder)));
	table->resizeColumnsToContents();
	vlay->addWidget(table);
	connect(table, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editButtonClicked()));
	
	connect(table->selectionModel(), &QItemSelectionModel::currentRowChanged,
		this, &TrainListWidget::onCurrentRowChanged);

	//auto* h = new ButtonGroup<2>({ "上移","下移"});
	//h->setMinimumWidth(50);
	//vlay->addLayout(h);
	//todo: connect

	auto* g = new ButtonGroup<4>({ "编辑","添加","删除","批量"});
	g->setMinimumWidth(50);
	vlay->addLayout(g);
	g->connectFront(SIGNAL(clicked()), this, {
		SLOT(editButtonClicked()),SIGNAL(addNewTrain()),SLOT(actRemoveTrains())
		});

	auto* menu = new QMenu(this);
	menu->addAction(tr("全选"), this, &TrainListWidget::selectAll);
	menu->addAction(tr("全不选"), this, &TrainListWidget::deselectAll);
	menu->addAction(tr("反选"), this, &TrainListWidget::selectInverse);
	menu->addSeparator();

	menu->addAction(tr("批量分类"), this, &TrainListWidget::batchChange);
	menu->addAction(tr("批量添加标签"), this, &TrainListWidget::actBatchAddTagBat);
	menu->addAction(tr("批量删除标签"), this, &TrainListWidget::actBatchRemoveTagBat);
	menu->addAction(tr("批量设置运行线样式"), this, &TrainListWidget::actChangePenBat);
	menu->addSeparator();
	menu->addAction(tr("按时刻表重设始发终到"), this, &TrainListWidget::actResetStartingTerminalFromTimetableBat);
	menu->addAction(tr("自动始发终到站适配"), this, &TrainListWidget::actAutoStartingTerminalBat);
	menu->addAction(tr("自动始发终到站适配 (放宽)"), this, &TrainListWidget::actAutoStartingTerminalLooserBat);
	menu->addAction(tr("自动推断列车类型"), this, &TrainListWidget::actAutoTrainTypeBat);
	menu->addAction(tr("自动设置运行线样式"), this, &TrainListWidget::actAutoTrainPenBat);
	menu->addAction(tr("取消自动运行线样式"), this, &TrainListWidget::actManualTrainPenBat);
	menu->addAction(tr("自动设置营业站"), this, &TrainListWidget::actAutoBusinessBat);
	menu->addAction(tr("自动更正时刻表 (测试)"), this, &TrainListWidget::actAutoCorrectionBat);
	menu->addSeparator();
	menu->addAction(tr("导出事件表 (csv)"), this, &TrainListWidget::actExportTrainEventListBat);
	menu->addAction(tr("导出时刻表 (csv)"), this, &TrainListWidget::actExportTrainTimetableBat);

	g->get(3)->setMenu(menu);

	setLayout(vlay);
}

QList<std::shared_ptr<Train>> TrainListWidget::batchOpSelectedTrains()
{
	auto lst = table->selectionModel()->selectedRows();
	QList<std::shared_ptr<Train>> res;
	foreach(const auto & t, lst) {
		res.push_back(coll.trainAt(t.row()));
	}
	if (res.empty()) {
		// 2025.09.09 CHANGE: add all shown rows if no selected
		qInfo() << "batchOpSelectedTrains: select all shown rows";
		for (int r = 0; r < model->rowCount({}); r++) {
			if (!table->isRowHidden(r)) {
				res.push_back(coll.trainAt(r));
			}
		}
		//QMessageBox::warning(this, tr("错误"), tr("批量操作：请先选择至少一个车次！"));
	}
	return res;
}

void TrainListWidget::searchTrain()
{
	const QString& s = editSearch->text();
	searchText = s;
	onFilterChanged();
}


void TrainListWidget::clearFilter()
{
	searchText.clear();
	onFilterChanged();
}

void TrainListWidget::editButtonClicked()
{
	auto index = table->currentIndex();
	if (index.isValid()) {
		emit editTrain(coll.trainAt(index.row()));
	}
}

void TrainListWidget::batchChange()
{
	auto lst = table->selectionModel()->selectedRows();
	QVector<int> rows;
	foreach (const auto& t , lst) {
		rows.push_back(t.row());
	}
	if (rows.empty()) {
		//QMessageBox::warning(this, tr("错误"), tr("批量设置列车类型：请先选择至少一个车次！"));
		qInfo() << "batchChange: automatically select all shown rows";
		for (int r = 0; r < model->rowCount({}); r++) {
			if (!table->isRowHidden(r)) {
				rows.push_back(r);
			}
		}
	}

	if (rows.empty()) {
		QMessageBox::warning(this, tr("错误"), tr("批量设置列车类型：请先选择至少一个车次！"));
		return;
	}

	auto types = coll.typeNames();
	bool ok;
	auto nty_s = QInputDialog::getItem(this, tr("批量设置类型"), tr("将选中的[%1]个车次类型设置为：").arg(rows.count()),
		types, 0, true, &ok);
	if (!ok)return;

	auto nty = coll.typeManager().find(nty_s);
	if (!nty) {
		// 2025.08.15: we should create the type first, if it has not been created
		nty = coll.typeManager().createType(nty_s);
		emit autoAddNewType(nty);
	}

	//操作压栈
	if (_undo) {
		_undo->push(new qecmd::BatchChangeType(coll, rows, nty, model));
	}
	else {
		// 2023.08.12: mind _undo may be null. For this case, commit directly
		for (int i = 0; i < rows.size(); i++) {
			int index = rows.at(i);
			auto train = coll.trainAt(index);
			train->setType(nty);
			coll.updateTrainType(train, nty);
		}
		model->commitBatchChangeType(rows);
	}
	QMessageBox::information(this, tr("提示"), tr("类型更新完毕。此功能不会自动重新铺画运行图，如果需要，"
		"请手动重新铺画或刷新运行图。"));
}

void TrainListWidget::batchAddTag(const QList<std::shared_ptr<Train>>& trains)
{
	auto tagNames = coll.tagManager().tagNames();
	bool ok;
	auto ntag_s = QInputDialog::getItem(this, tr("批量设置标签"),
		tr("为选中的[%1]个车次设置以下标签：").arg(trains.count()),
		tagNames, 0, true, &ok);
	if (!ok)
		return;

	std::vector<std::shared_ptr<Train>> trains_sel;
	auto tag = coll.tagManager().find(ntag_s);  // possible null
	std::ranges::move(std::ranges::views::filter(trains, [&tag](const std::shared_ptr<Train>& t) {
		return !tag || !t->hasTag(tag);
		}), std::back_inserter(trains_sel));

	if (trains_sel.empty()) {
		QMessageBox::information(this, tr("提示"), tr("所选列车均已含有所给标签，操作没有效果"));
	}
	else {
		// Emit and let train context handle this
		emit batchAddTagApplied(ntag_s, trains_sel);
	}
}

void TrainListWidget::batchRemoveTag(const QList<std::shared_ptr<Train>>& trains)
{
	auto tagNames = coll.tagManager().tagNames();
	bool ok;
	auto ntag_s = QInputDialog::getItem(this, tr("批量设置标签"),
		tr("为选中的[%1]个车次移除以下标签（如果含有）：").arg(trains.count()),
		tagNames, 0, true, &ok);
	if (!ok)
		return;

	auto tag = coll.tagManager().find(ntag_s);  // possible null
	if (!tag) {
		QMessageBox::information(this, tr("提示"), tr("所有选中车次皆不含所给标签，操作没有效果"));
	}

	std::vector<std::pair<std::shared_ptr<Train>, int>> remove_data;
	for (auto t : trains) {
		if (auto idx = t->tagIndex(tag); idx >= 0) {
			remove_data.emplace_back(t, idx);
		}
	}

	if (remove_data.empty()) {
		QMessageBox::information(this, tr("提示"), tr("所选列车均不含有所给标签，操作没有效果"));
	}
	else {
		// Emit and let train context handle this
		emit batchRemoveTagApplied(tag, remove_data);
	}
}

void TrainListWidget::actBatchAddTagBat()
{
	auto trains = batchOpSelectedTrains();
	if (trains.empty()) {
		QMessageBox::warning(this, tr("错误"), tr("批量添加标签：未选择车次！"));
	}

	batchAddTag(trains);
}

void TrainListWidget::actBatchRemoveTagBat()
{
	auto trains = batchOpSelectedTrains();
	if (trains.empty()) {
		QMessageBox::warning(this, tr("错误"), tr("批量移除列车标签：请先选择至少一个车次！"));
		return;
	}

	batchRemoveTag(trains);
}


void TrainListWidget::actRemoveTrains()
{
	auto lst = table->selectionModel()->selectedRows(0);
	if (lst.empty())
		return;

	QList<int> rows;
	rows.reserve(lst.size());
	for (const auto& p : lst) {
		rows.append(p.row());
	}

	std::sort(rows.begin(), rows.end());
	QList<std::shared_ptr<Train>> trains;
	trains.reserve(rows.size());

	//注意实际的删除由Model执行。这里生成列表，发给Main来执行CMD压栈
	for (auto p = rows.rbegin(); p != rows.rend(); ++p) {
		std::shared_ptr<Train> train(coll.trainAt(*p));   //move 
		trains.prepend(train);
	}

	if (_undo) {
		// 2023.08.16: move the en-stack operation to trainContext
		emit removeTrains(trains, rows);
		//_undo->push(new qecmd::RemoveTrains(trains, rows, coll, model));
	}
	else {
		//不支持撤销，就直接执行了
		model->redoRemoveTrains(trains, rows);
	}
}

void TrainListWidget::onCurrentRowChanged(const QModelIndex& idx)
{
	if (!idx.isValid())
		return;
	int row = idx.row();
	emit currentTrainChanged(coll.trainAt(row));
}

void TrainListWidget::resetStartingTerminalFromTimetable(const QList<std::shared_ptr<Train>>& trains)
{
	QString text = tr("遍历所有车次，将始发站、终到站分别改为时刻表第一站和最后一站。"
		"是否继续？");
	auto flag = QMessageBox::question(this, tr("重设始发终到站"), text);
	if (flag != QMessageBox::Yes)
		return;
	qecmd::StartingTerminalData data;
	foreach(auto train, trains) {
		if (train->empty())
			continue;
		if (const auto& fn = train->timetable().front().name; train->starting() != fn) {
			data.startings.emplace_back(std::make_pair(train, fn));
		}
		if (const auto& ln = train->timetable().back().name; train->terminal() != ln) {
			data.terminals.emplace_back(std::make_pair(train, ln));
		}
	}
	if (data.startings.empty() && data.terminals.empty()) {
		QMessageBox::information(this, tr("提示"), tr("没有车次受到影响。"));
	}
	else {
		QMessageBox::information(this, tr("提示"), tr("设置成功。影响%1个车次的始发站和"
			"%2个车次的终到站。").arg(data.startings.size()).arg(data.terminals.size()));
		//undoStack->push(new qecmd::AutoStartingTerminal(std::move(data), contextTrain));
		emit batchChangeStartingTerminal(data);
	}
}

void TrainListWidget::actResetStartingTerminalFromTimetableBat()
{
	auto lst = batchOpSelectedTrains();
	if (!lst.empty()) {
		resetStartingTerminalFromTimetable(lst);
	}
}

void TrainListWidget::autoStartingTerminal(const QList<std::shared_ptr<Train>>& trains)
{
	QString text = tr("遍历所有车次，当车次满足以下条件时，将该车次的始发站调整为在本线的第一个车站，终到站调整为" \
		"在本线的最后一个车站。\n"
		"（1）该车次在时刻表中的第一个（最后一个）站铺画在运行图上；\n"
		"（2）该车次时刻表中第一个（最后一个）站站名以始发（终到）站站名开头，"
		"“场”字结尾；\n"
		"（3）始发（终到）站及时刻表中第一个（最后一个）站站名不含域解析符（::）。\n"
		"如果希望对没有铺画的列车也执行这个操作，请使用[自动始发终到适配 (放宽)]功能。"
		"是否继续？");
	auto flag = QMessageBox::question(this, tr("自动始发终到站适配"), text);
	if (flag != QMessageBox::Yes)
		return;
	qecmd::StartingTerminalData data;
	foreach(auto train, trains) {
		if (train->empty())
			continue;
		auto* first = train->boundFirst();
		if (first) {
			const auto& firstname = first->trainStation->name;
			const auto& starting = train->starting();
			if (firstname.isSingleName() && starting.isSingleName() &&
				firstname.station().startsWith(starting.station()) &&
				firstname.station().endsWith(tr("场"))) {
				data.startings.emplace_back(std::make_pair(train, firstname));
			}
		}
		auto* last = train->boundLast();
		if (last) {
			const auto& lastname = last->trainStation->name;
			const auto& terminal = train->terminal();
			if (lastname.isSingleName() && terminal.isSingleName() &&
				lastname.station().startsWith(terminal.station()) &&
				lastname.station().endsWith(tr("场"))) {
				data.terminals.emplace_back(std::make_pair(train, lastname));
			}
		}
	}

	if (data.startings.empty() && data.terminals.empty()) {
		QMessageBox::information(this, tr("提示"), tr("没有车次受到影响。"));
	}
	else {
		QMessageBox::information(this, tr("提示"), tr("设置成功。影响%1个车次的始发站和"
			"%2个车次的终到站。").arg(data.startings.size()).arg(data.terminals.size()));
		//undoStack->push(new qecmd::AutoStartingTerminal(std::move(data), contextTrain));
		emit batchChangeStartingTerminal(data);
	}
}


void TrainListWidget::autoStartingTerminalLooser(const QList<std::shared_ptr<Train>>& trains)
{
	QString text = tr("遍历所有车次，当车次满足以下条件时，将该车次的始发站调整为在本线的第一个车站，终到站调整为" \
		"在本线的最后一个车站。\n"
		"（1）该车次时刻表中第一个（最后一个）站站名以始发（终到）站站名开头，"
		"“场”字结尾；\n"
		"（2）始发（终到）站及时刻表中第一个（最后一个）站站名不含域解析符（::）。\n"
		"此功能与[自动始发终到站适配]的区别在不要求始发终到站铺画在运行图上。"
		"是否继续？");
	auto flag = QMessageBox::question(this, tr("自动始发终到站适配"), text);
	if (flag != QMessageBox::Yes)
		return;
	qecmd::StartingTerminalData data;
	foreach(auto train, trains) {
		if (train->empty())
			continue;
		auto first = train->firstStation();
		const auto& firstname = first->name;
		const auto& starting = train->starting();
		if (firstname.isSingleName() && starting.isSingleName() &&
			firstname.station().startsWith(starting.station()) &&
			firstname.station().endsWith(tr("场"))) {
			data.startings.emplace_back(std::make_pair(train, firstname));
		}

		auto last = train->lastStation();

		const auto& lastname = last->name;
		const auto& terminal = train->terminal();
		if (lastname.isSingleName() && terminal.isSingleName() &&
			lastname.station().startsWith(terminal.station()) &&
			lastname.station().endsWith(tr("场"))) {
			data.terminals.emplace_back(std::make_pair(train, lastname));
		}

	}

	if (data.startings.empty() && data.terminals.empty()) {
		QMessageBox::information(this, tr("提示"), tr("没有车次受到影响。"));
	}
	else {
		QMessageBox::information(this, tr("提示"), tr("设置成功。影响%1个车次的始发站和"
			"%2个车次的终到站。").arg(data.startings.size()).arg(data.terminals.size()));
		//undoStack->push(new qecmd::AutoStartingTerminal(std::move(data), contextTrain));
		emit batchChangeStartingTerminal(data);
	}
}

void TrainListWidget::autoTrainType(const QList<std::shared_ptr<Train>>& trains)
{
	auto flag = QMessageBox::question(this, tr("提示"), tr("此操作根据当前的列车类型管理器，"
		"对所有列车，根据其全车次，使用正则表达式匹配到列车类型。\n如果以前有手动设置的列车"
		"类型，数据将被覆盖。\n是否确认？"));
	if (flag != QMessageBox::Yes) return;

	qecmd::AutoTrainType::data_t data;

	foreach(auto train, trains) {
		auto type = coll.typeManager().fromRegex(train->trainName());
		if (type != train->type()) {
			data.emplace_back(train, type);
		}
	}
	if (data.empty()) {
		QMessageBox::information(this, tr("提示"), tr("自动类型推断应用完成，"
			"没有产生任何变化。"));
	}
	else {
		int sz = data.size();
		//undoStack->push(new qecmd::AutoTrainType(std::move(data), contextTrain));
		emit batchAutoChangeType(data);
		QMessageBox::information(this, tr("提示"), tr("自动类型推断应用完成，"
			"%1个车次的类型发生变化。如有问题，可以撤销。").arg(sz));
	}
}

void TrainListWidget::actAutoStartingTerminalBat()
{
	const auto& lst = batchOpSelectedTrains();
	if (!lst.empty()) {
		autoStartingTerminal(lst);
	}
}

void TrainListWidget::actAutoStartingTerminalLooserBat()
{
	const auto& lst = batchOpSelectedTrains();
	if (!lst.empty()) {
		autoStartingTerminalLooser(lst);
	}
}

void TrainListWidget::actAutoTrainTypeBat()
{
	const auto& lst = batchOpSelectedTrains();
	if (!lst.empty()) {
		autoTrainType(lst);
	}
}

void TrainListWidget::actAutoTrainPenBat()
{
	const auto& lst = batchOpSelectedTrains();
	std::deque<std::shared_ptr<Train>> trains;
	foreach(auto t, lst) {
		if (!t->autoPen()) {
			trains.emplace_back(t);
		}
	}
	if (!trains.empty()) {
		emit batchAutoPen(trains);
	}
}

void TrainListWidget::actManualTrainPenBat()
{
	const auto& lst = batchOpSelectedTrains();
	std::deque<std::shared_ptr<Train>> trains;
	foreach(auto t, lst) {
		if (t->autoPen()) {
			trains.emplace_back(t);
		}
	}
	if (!trains.empty()) {
		emit batchManualPen(trains);
	}
}

void TrainListWidget::actChangePenBat()
{
	auto lst = batchOpSelectedTrains();
	std::deque<std::shared_ptr<Train>> trains(std::make_move_iterator(lst.begin()), std::make_move_iterator(lst.end()));
	if (!trains.empty()) {
		auto res = TrainPenWidget::getPen(this, trains.front(), tr("批量设置运行线"),
			tr("请为所选的车次设置运行线样式"));
		if (!res.accepted)
			return;
		emit batchChangePen(std::move(trains), res.pen);
	}
}

void TrainListWidget::actExportTrainEventListBat()
{
	auto lst = batchOpSelectedTrains();
	if (!lst.empty()) {
		emit batchExportTrainEventList(lst);
	}
}

void TrainListWidget::actExportTrainTimetableBat()
{
	auto lst = batchOpSelectedTrains();
	if (!lst.empty()) {
		exportTrainTimetable(lst);
	}
}

void TrainListWidget::exportTrainTimetable(const QList<std::shared_ptr<Train>>& trains)
{
	auto res = QMessageBox::question(this, tr("导出时刻表"),
		tr("此操作将所选列车时刻表导出为CSV表格，所有数据导出至同一个文件。是否继续？"));
	if (res != QMessageBox::Yes)
		return;

	auto filename = QFileDialog::getSaveFileName(this,
		tr("导出时刻表"), {}, tr("CSV文件 (*.csv)\n所有文件 (*)"));
	if (filename.isEmpty())return;

	QFile file(filename);
	file.open(QFile::WriteOnly);
	if (!file.isOpen())
		return;
	QTextStream sout(&file);

	foreach (const auto& train, trains) {
		// 车次，站名，到点，开点，股道，备注
		for (const auto& st : train->timetable()) {
			sout << train->trainName().full() << ',' <<
				st.name.toSingleLiteral() << ',' <<
				st.arrive.toString(TrainTime::HMS) << ',' <<
				st.depart.toString(TrainTime::HMS) << ',' <<
				st.track << ',' <<
				st.note << '\n';
		}
	}

	file.close();
}

void TrainListWidget::actAutoBusinessBat()
{
	auto lst = batchOpSelectedTrains();
	if (!lst.empty()) {
		emit batchAutoBusiness(lst);
	}
}

void TrainListWidget::actAutoCorrectionBat()
{
	auto lst = batchOpSelectedTrains();
	if (!lst.empty()) {
		emit batchAutoCorrect(lst);
	}
}

void TrainListWidget::selectAll()
{
	auto* sel = table->selectionModel();
	sel->select(QItemSelection(model->index(0, 0),
		model->index(coll.trainCount() - 1, TrainListModel::MAX_COLUMNS - 1)), QItemSelectionModel::Select);
}

void TrainListWidget::deselectAll()
{
	auto* sel = table->selectionModel();
	sel->select(QItemSelection(model->index(0, 0),
		model->index(coll.trainCount() - 1, TrainListModel::MAX_COLUMNS - 1)), QItemSelectionModel::Deselect);
}

void TrainListWidget::selectInverse()
{
	auto* sel = table->selectionModel();
	sel->select(QItemSelection(model->index(0, 0),
		model->index(coll.trainCount() - 1, TrainListModel::MAX_COLUMNS - 1)), QItemSelectionModel::Toggle);
}

void TrainListWidget::onFilterChanged()
{
	auto* f = filter->filter();
	
	// Update the filter data, also consider the search text
	for (int i = 0; i < coll.trainCount(); i++) {
		const auto& t = coll.trainAt(i);
		bool selected = searchText.isEmpty() || t->trainName().contains(searchText);
		if (selected) {
			if (f && !f->check(t)) {
				selected = false;
			}
		}

		table->setRowHidden(i, !selected);
	}
}

void TrainListWidget::actResetStartingTerminalFromTimetableAll()
{
	resetStartingTerminalFromTimetable(coll.trains());
}

void TrainListWidget::actAutoStartingTerminalAll()
{
	autoStartingTerminal(coll.trains());
}

void TrainListWidget::actAutoStartingTerminalLooserAll()
{
	autoStartingTerminalLooser(coll.trains());
}

void TrainListWidget::actAutoTrainTypeAll()
{
	autoTrainType(coll.trains());
}

void TrainListWidget::actExportTrainEventListAll()
{
	exportTrainTimetable(coll.trains());
}


#endif

