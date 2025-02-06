#include "trainfiltercombo.h"
#include "data/train/predeftrainfiltercore.h"
#include "data/train/traincollection.h"

TrainFilterCombo::TrainFilterCombo(TrainCollection &coll, QWidget *parent):
    QComboBox(parent),coll(coll)
{
    instances.emplace(this);
    refreshData();
    connect(this,qOverload<int>(&QComboBox::currentIndexChanged),
            this,&TrainFilterCombo::onIndexChanged);
}

TrainFilterCombo::~TrainFilterCombo()
{
    instances.erase(this);
}

void TrainFilterCombo::onIndexChanged(int id)
{
    const auto& it=itemData(id);
    if (it.type() == QMetaType::Int){   // For Qt6, we should use typeId(); However, for compatiblity with Qt5, keep type() for a moment
        // for Sys filters
        auto t=static_cast<PredefTrainFilterCore::SysFilterId>(it.toInt());
        auto* f=PredefTrainFilterCore::getSysFilter(t);
        _current=f;
    }else{
        _current=qvariant_cast<PredefTrainFilterCore*>(it);
    }
    emit filterChanged(_current);
}

void TrainFilterCombo::refreshData()
{
    clear();
    addItem(tr("(无预设)"));

    for (int i=0;i<PredefTrainFilterCore::MAX_FILTERS;i++){
        addItem(PredefTrainFilterCore::getSysFilterName(
                    static_cast<PredefTrainFilterCore::SysFilterId>(i)),
                QVariant::fromValue(i));
    }

    for (const auto& t:coll.filters()){
        addItem(t->name(), QVariant::fromValue(t.get()));
    }
}

std::set<TrainFilterCombo*> TrainFilterCombo::instances{};

void TrainFilterCombo::refreshAll()
{
    for (auto* t: instances){
        t->refreshData();
    }
}
