#include "generaldoublespindelegate.h"

#include <QDoubleSpinBox>

GeneralDoubleSpinDelegate::GeneralDoubleSpinDelegate(QObject *parent, int decimals) :
    QStyledItemDelegate(parent),_decimals(decimals)
{

}

QWidget *GeneralDoubleSpinDelegate::createEditor(QWidget *parent,
                    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto* sp=new QDoubleSpinBox(parent);
    setupSpin(sp);
    return sp;
}

void GeneralDoubleSpinDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto* ed=static_cast<QDoubleSpinBox*>(editor);
    ed->setValue(index.data(Qt::EditRole).toDouble());
}

void GeneralDoubleSpinDelegate::setModelData(QWidget *editor,
                        QAbstractItemModel *model, const QModelIndex &index) const
{
    auto* ed=static_cast<QDoubleSpinBox*>(editor);
    model->setData(index,ed->value(),Qt::EditRole);
}

QString GeneralDoubleSpinDelegate::displayText(const QVariant &value,
                                            const QLocale &locale) const
{
    Q_UNUSED(locale);
    return QString::number(value.toDouble(),'f',_decimals);
}

void GeneralDoubleSpinDelegate::setupSpin(QDoubleSpinBox *sp) const
{
    sp->setRange(-1000000,100000);
    sp->setDecimals(_decimals);
}
