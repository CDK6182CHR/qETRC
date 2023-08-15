#include "pathstationdelegate.h"
#include "model/trainpath/pathmodel.h"
#include "data/rail/railway.h"

#include <QComboBox>

PathStationDelegate::PathStationDelegate(PathModel *model, QObject *parent)
    : QStyledItemDelegate{parent}, pathModel(model)
{
}

QWidget *PathStationDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto* cb=new QComboBox(parent);
    cb->setEditable(true);
    return cb;
}

void PathStationDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto* cb=qobject_cast<QComboBox*>(editor);
    cb->clear();
    auto cur_st=index.data(Qt::EditRole).toString();

    auto rail=pathModel->getRailwayForRow(index.row());
    if (rail){
        int idx=-1;
        // Set the stations of the railway
        int i=0;
        foreach(auto st, rail->stations()){
            auto stname = st->name.toSingleLiteral();
            cb->addItem(stname);
            if (stname == cur_st){
                idx = i;
            }
            i++;
        }
        if (idx >=0){
            cb->setCurrentIndex(idx);
        }
    }
}

void PathStationDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto* cb=qobject_cast<QComboBox*>(editor);
    const auto& prev_st=index.data(Qt::EditRole);
    auto cur_text=cb->currentText();
    bool updated= (prev_st.toString() != cur_text);
    model->setData(index, cur_text, Qt::EditRole);
    if (updated){
        pathModel->onStationChanged(index);
    }
}
