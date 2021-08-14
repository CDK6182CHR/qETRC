#include "postivespindelegate.h"
#include <QSpinBox>

PostiveSpinDelegate::PostiveSpinDelegate(int step_, QObject *parent):
    QStyledItemDelegate(parent),step(step_)
{

}

QWidget *PostiveSpinDelegate::createEditor(QWidget *parent,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    Q_UNUSED(option);
    if(!index.isValid())return nullptr;
    auto* spin=new QSpinBox(parent);
    spin->setRange(0,1000000);
    spin->setSingleStep(step);
    connect(spin, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
    return spin;
}

void PostiveSpinDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto* sp=static_cast<QSpinBox*>(editor);
    sp->setValue(qvariant_cast<int>(index.data(Qt::EditRole)));
}

void PostiveSpinDelegate::setModelData(QWidget *editor,
                                       QAbstractItemModel *model, const QModelIndex &index) const
{
    auto* sp=static_cast<QSpinBox*>(editor);
    model->setData(index,sp->value(),Qt::EditRole);
}

void PostiveSpinDelegate::onValueChanged()
{
    emit commitData(qobject_cast<QWidget*>(sender()));
}
