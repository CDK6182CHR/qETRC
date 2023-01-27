#include "selecttraintypelistwidget.h"

#include <QStandardItemModel>
#include <QVBoxLayout>

#include "data/train/traincollection.h"
#include "data/train/traintype.h"
#include "model/delegate/qedelegate.h"
#include "data/common/qeglobal.h"   // for qHash

SelectTrainTypeListWidget::SelectTrainTypeListWidget(TrainCollection &coll_, QWidget *parent):
    QListView(parent), coll(coll_), model(new QStandardItemModel(this))
{
    initUI();
}

QSet<std::shared_ptr<const TrainType> > SelectTrainTypeListWidget::selected() const
{
    QSet<std::shared_ptr<const TrainType>> res;
    auto&& sel=selectionModel()->selectedRows();
    foreach(const auto& p,sel){
        auto tp= qvariant_cast<std::shared_ptr<TrainType>>(p.data(qeutil::TrainTypeRole));
        res.insert(tp);
    }
    return res;
}

void SelectTrainTypeListWidget::initUI()
{
    setModel(model);
    setSelectionMode(QListView::MultiSelection);
}

void SelectTrainTypeListWidget::refreshTypes()
{
    auto _selected=selected();
    refreshTypesWithSelection(_selected);
}

void SelectTrainTypeListWidget::refreshTypesWithSelection(
        const QSet<std::shared_ptr<const TrainType>> &_selected)
{
    const auto& cnt=coll.typeCount();
    model->setRowCount(cnt.count());
    auto* sel=selectionModel();
    int row=0;
    for(auto p=cnt.begin();p!=cnt.end();++p){
        auto* it=new QStandardItem(p.key()->name());
        QVariant v;
        v.setValue(p.key());
        it->setData(v,qeutil::TrainTypeRole);
        model->setItem(row,it);
        if (_selected.contains(p.key())){
            sel->select(model->index(row,0),QItemSelectionModel::Select);
        }else{
            sel->select(model->index(row,0),QItemSelectionModel::Deselect);
        }
        row++;
    }
}

void SelectTrainTypeListWidget::clearSelected()
{
    refreshTypesWithSelection({});
}
