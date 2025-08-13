#include "trainfilterbasicwidget.h"
#include "data/train/trainfiltercore.h"
#include "selecttraintypelistwidget.h"
#include "trainnameregextable.h"
#include "editors/rail/stationnameregextable.h"
#include "selectroutinglistwidget.h"
#include "traintagselecttable.h"
#include "data/train/traincollection.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QTextBrowser>

#include "util/qefoldwidget.h"

TrainFilterBasicWidget::TrainFilterBasicWidget(TrainCollection &coll,
                                               TrainFilterCore* core, QWidget *parent)
    : QWidget{parent}, coll(coll), _core(core)
{
    initUI();
}

void TrainFilterBasicWidget::appliedData(TrainFilterCore* core)
{
    core->useType = ckType->isChecked();
    if (core->useType) {
        core->types = lstType->selected();
    }
    core->useInclude = ckInclude->isChecked();
    if (core->useInclude) {
        core->includes = tabInclude->names();
    }
    core->useExclude = ckExclude->isChecked();
    if (core->useExclude) {
        core->excludes = tabExclude->names();
    }
	core->useIncludeTag = ckIncludeTag->isChecked();
    if (core->useIncludeTag) {
        core->includeTags = tabIncludeTag->selectedTags();
    }
	core->useExcludeTag = ckExcludeTag->isChecked();
    if (core->useExcludeTag) {
        core->excludeTags = tabExcludeTag->selectedTags();
	}

    core->useRouting = ckRouting->isChecked();
    if (core->useRouting) {
        auto&& res = lstRouting->result();
        core->routings = res.first;
        core->selNullRouting = res.second;
    }
    core->useStarting = ckStarting->isChecked();
    if (core->useStarting) {
        core->startings = tabStarting->names();
    }
    core->useTerminal = ckTerminal->isChecked();
    if (core->useTerminal) {
        core->terminals = tabTerminal->names();
    }

    core->showOnly = ckShowOnly->isChecked();
    core->useInverse = ckInverse->isChecked();

    // 客车类型  Auto表示都行
    core->passengerType = TrainPassenger::Auto;
    if (gpPassen->get(0)->isChecked())
        core->passengerType = TrainPassenger::True;
    else if (gpPassen->get(1)->isChecked())
        core->passengerType = TrainPassenger::False;
}

void TrainFilterBasicWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);

    ckType=new QCheckBox(tr("列车种类"));
    lstType=new SelectTrainTypeListWidget(coll);
    auto* fold=new QEFoldWidget(ckType, lstType);
    fold->expand();
    vlay->addWidget(fold);

    ckInclude = new QCheckBox(tr("包含车次"));
    tabInclude=new TrainNameRegexTable(coll);
    fold=new QEFoldWidget(ckInclude,tabInclude);
    vlay->addWidget(fold);

    ckExclude=new QCheckBox(tr("排除车次"));
    tabExclude=new TrainNameRegexTable(coll);
    fold=new QEFoldWidget(ckExclude,tabExclude);
    vlay->addWidget(fold);

	ckIncludeTag = new QCheckBox(tr("含有标签"));
	tabIncludeTag = new TrainTagSelectTable(coll.tagManager());
	fold = new QEFoldWidget(ckIncludeTag, tabIncludeTag);
    vlay->addWidget(fold);

	ckExcludeTag = new QCheckBox(tr("不含标签"));
	tabExcludeTag = new TrainTagSelectTable(coll.tagManager());
	fold = new QEFoldWidget(ckExcludeTag, tabExcludeTag);
	vlay->addWidget(fold);

    ckRouting = new QCheckBox(tr("属于交路"));
    lstRouting = new SelectRoutingListWidget(coll);
    fold = new QEFoldWidget(ckRouting, lstRouting);
    vlay->addWidget(fold);

    ckStarting = new QCheckBox(tr("始发于所选站"));
    tabStarting = new StationNameRegexTable;
    fold = new QEFoldWidget(ckStarting, tabStarting);
    vlay->addWidget(fold);

    ckTerminal = new QCheckBox(tr("终到于所选站"));
    tabTerminal = new StationNameRegexTable;
    fold = new QEFoldWidget(ckTerminal, tabTerminal);
    vlay->addWidget(fold);

    auto* flay = new QFormLayout;
    gpPassen=new RadioButtonGroup<3>({"客车","非客车","全部"},this);
    gpPassen->get(2)->setChecked(true);
    flay->addRow(tr("是否客车"),gpPassen);

    ckShowOnly = new QCheckBox(tr("启用"));
    flay->addRow(tr("仅包括当前显示车次"),ckShowOnly);

    ckInverse = new QCheckBox(tr("启用"));
    flay->addRow(tr("反向选择"),ckInverse);
    vlay->addLayout(flay);
    vlay->addStretch(0);
}

void TrainFilterBasicWidget::actApply()
{
    appliedData(_core);
}

bool TrainFilterBasicWidget::clearFilter()
{
    auto res=QMessageBox::question(this,tr("清空筛选器"),
                                   tr("清空当前列车筛选条件，设置所有列车被选中。是否确认？"));
    if (res!=QMessageBox::Yes){
        return false;
    }
    clearNotChecked();
    return true;
}

void TrainFilterBasicWidget::clearNotChecked()
{
    ckType->setChecked(false);
    ckInclude->setChecked(false);
    ckExclude->setChecked(false);
	ckIncludeTag->setChecked(false);
    ckExcludeTag->setChecked(false);
    ckRouting->setChecked(false);
    ckShowOnly->setChecked(false);
    ckInverse->setChecked(false);
    ckStarting->setChecked(false);
    ckTerminal->setChecked(false);
    gpPassen->get(2)->setChecked(true);
}

void TrainFilterBasicWidget::refreshData()
{
    refreshDataWith(this->_core);
}

void TrainFilterBasicWidget::refreshDataWith(const TrainFilterCore *core)
{
    if (!core){
        clearNotChecked();
        return;
    }
    lstType->refreshTypesWithSelection(core->types);
    tabInclude->refreshData(core->includes);
    tabExclude->refreshData(core->excludes);
	tabIncludeTag->refreshData(core->includeTags);
	tabExcludeTag->refreshData(core->excludeTags);
    lstRouting->refreshRoutingsWithSelection(std::make_pair(core->routings, core->selNullRouting));
    tabStarting->refreshData(core->startings);
    tabTerminal->refreshData(core->terminals);

    ckType->setChecked(core->useType);
    ckInclude->setChecked(core->useInclude);
    ckExclude->setChecked(core->useExclude);
	ckIncludeTag->setChecked(core->useIncludeTag);
	ckExcludeTag->setChecked(core->useExcludeTag);
    ckRouting->setChecked(core->useRouting);
    ckStarting->setChecked(core->useStarting);
    ckTerminal->setChecked(core->useTerminal);

    ckShowOnly->setChecked(core->showOnly);
    ckInverse->setChecked(core->useInverse);
    {
        int passen_id = 2;
        switch (core->passengerType)
        {
        case TrainPassenger::False:passen_id = 1;
            break;
        case TrainPassenger::Auto:passen_id = 2;
            break;
        case TrainPassenger::True:passen_id = 0;
            break;
        default:
            break;
        }
        gpPassen->get(passen_id)->setChecked(true);
    }
}


