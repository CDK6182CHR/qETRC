#include "pathdelegates.h"

#include <cassert>
#include <QComboBox>

#include "data/rail/railcategory.h"
#include "data/rail/railway.h"
#include "model/trainpath/pathmodel.h"
#include "model/delegate/qedelegate.h"

PathRailwayDelegate::PathRailwayDelegate(RailCategory &cat, PathModel *model, QObject *parent):
    QStyledItemDelegate(parent), railcat(cat), model(model)
{

}

QWidget *PathRailwayDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    auto* ed=new QComboBox(parent);
    return ed;
}

void PathRailwayDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto* cb=qobject_cast<QComboBox*>(editor);
    cb->clear();

    const auto& cur_name = index.data().toString();

    if (index.row() == 0){
        // For the first row, any rail is available
        setComboUnconstrained(cb, cur_name);
    }else{
        setComboConstrained(cb, cur_name, index.row());
    }
}

void PathRailwayDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                       const QModelIndex &index) const
{
    auto* cb=qobject_cast<QComboBox*>(editor);
    int idx=cb->currentIndex();
    if (idx == -1){
        // No valid railway!
        model->setData(index, {}, qeutil::RailwayRole);
        model->setData(index, {}, Qt::DisplayRole);
    }else{
        model->setData(index, cb->currentData(), qeutil::RailwayRole);
        model->setData(index, cb->currentText(), Qt::DisplayRole);
    }
    // Other check/updates may needed here
}

void PathRailwayDelegate::setComboUnconstrained(QComboBox *cb, const QString &cur_name) const
{
    cb->clear();

    int idx=0;
    int cur_idx=-1;
    foreach(auto rail, railcat.railways()){
        QVariant v;
        v.setValue(rail);
        cb->addItem(rail->name(), v);
        if (cur_name == rail->name()){
            cur_idx=idx;
        }
        idx++;
    }

    if (cur_idx >= 0){
        cb->setCurrentIndex(cur_idx);
    }
}

void PathRailwayDelegate::setComboConstrained(QComboBox *cb, const QString &cur_name, int row) const
{
    cb->clear();
    int cur_idx=-1;
    assert(row > 0);
    const auto& start_station_name=model->item(row-1, PathModel::ColEnd)->text();
    StationName start_station(start_station_name);

    for (int idx=0;idx<railcat.railways().size();idx++){
        auto rail=railcat.railways().at(idx);
        if (rail->containsStation(start_station)){
            // This is a valid attendee
            QVariant v;
            v.setValue(rail);
            cb->addItem(rail->name(), v);
            if (cur_name == rail->name()){
                cur_idx = idx;
            }
        }
    }

    if (cur_idx >= 0){
        cb->setCurrentIndex(cur_idx);
    }

}
