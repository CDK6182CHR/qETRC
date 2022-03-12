#include "generalspindelegate.h"
#include <QSpinBox>
#include <util/utilfunc.h>

GeneralSpinDelegate::GeneralSpinDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

QWidget *GeneralSpinDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    if(!index.isValid())return nullptr;
    auto* spin=new QSpinBox(parent);
    setupSpin(spin);
    return spin;
}

void GeneralSpinDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto* sp=static_cast<QSpinBox*>(editor);
    sp->setValue(qvariant_cast<int>(index.data(Qt::EditRole)));
}

void GeneralSpinDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto* sp=static_cast<QSpinBox*>(editor);
    model->setData(index,sp->value(),Qt::EditRole);
}

void GeneralSpinDelegate::setupSpin(QSpinBox *sp) const
{
    Q_UNUSED(sp);
}

SteppedSpinDelegate::SteppedSpinDelegate(int step, QObject *parent):
    GeneralSpinDelegate(parent),_step(step)
{

}

void SteppedSpinDelegate::setupSpin(QSpinBox *sp) const
{
    sp->setRange(-1000000,10000000);
    sp->setSingleStep(_step);
}

SecondSpinDelegate::SecondSpinDelegate(QObject *parent):
    GeneralSpinDelegate(parent)
{

}

void SecondSpinDelegate::setupSpin(QSpinBox *sp) const
{
    sp->setRange(0,59);
    sp->setSingleStep(10);
    sp->setWrapping(true);
}

void TrainGapSpinDelegate::setupSpin(QSpinBox *sp) const
{
    sp->setRange(0,10*3600);
    sp->setSingleStep(30);
    sp->setSuffix(tr(" 秒 (s)"));
}

QString TrainGapSpinDelegate::displayText(const QVariant &value,
                                          const QLocale &locale) const
{
    Q_UNUSED(locale);
    return qeutil::secsDiffToString(value.toInt());
}
