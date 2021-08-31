#include "linestyledelegate.h"

#include "util/linestylecombo.h"

LineStyleDelegate::LineStyleDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

QWidget *LineStyleDelegate::createEditor(QWidget *parent,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    auto* cb=new PenStyleCombo(parent);
    return cb;
}

void LineStyleDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto* cb=static_cast<PenStyleCombo*>(editor);
    cb->setCurrentIndex(index.data(Qt::EditRole).toInt());
}

void LineStyleDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    auto* cb=static_cast<PenStyleCombo*>(editor);
    model->setData(index, cb->currentIndex(), Qt::EditRole);
}
