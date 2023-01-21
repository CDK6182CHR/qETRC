#include "predeftrainfilterlist.h"
#include "predeftrainfiltermanager.h"
#include "predeftrainfilterwidget.h"
#include "data/train/predeftrainfiltercore.h"   // essential for the connection with pointers

PredefTrainFilterManager::PredefTrainFilterManager(TrainCollection &coll, QWidget *parent):
    QSplitter(parent),coll(coll)
{
    initUI();
    setWindowTitle(tr("预置列车筛选器"));
}

void PredefTrainFilterManager::initUI()
{
    lstWidget=new PredefTrainFilterList(coll);
    editWidget=new PredefTrainFilterWidget(coll);

    addWidget(lstWidget);
    addWidget(editWidget);

    connect(lstWidget, &PredefTrainFilterList::currentChanged,
        editWidget, &PredefTrainFilterWidget::setCore);

    // forward signals
    connect(lstWidget,&PredefTrainFilterList::addFilter,
            this,&PredefTrainFilterManager::addFilter);
    connect(lstWidget,&PredefTrainFilterList::removeFilter,
            this,&PredefTrainFilterManager::removeFilter);
    connect(editWidget, &PredefTrainFilterWidget::changeApplied,
        this, &PredefTrainFilterManager::updateFilter);
}

void PredefTrainFilterManager::commitAddFilter(int place, const PredefTrainFilterCore *filter)
{
    lstWidget->refreshList();
}

void PredefTrainFilterManager::commitRemoveFilter(int place, const PredefTrainFilterCore *filter)
{
    lstWidget->refreshList();
    if (editWidget->getCore()==filter){
        editWidget->setCore(nullptr);
    }
}

void PredefTrainFilterManager::commitUpdateFilter(const PredefTrainFilterCore* filter)
{
    lstWidget->refreshList();   // in case names may changed
    if (editWidget->getCore() == filter) {
        editWidget->refreshData();
    }
}

void PredefTrainFilterManager::refreshData()
{
    lstWidget->refreshList();
    editWidget->refreshData();
}
