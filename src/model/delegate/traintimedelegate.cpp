#include "traintimedelegate.h"

#include "qedelegate.h"
#include "util/traintimeedit.h"
#include "data/diagram/diagramoptions.h"

TrainTimeDelegate::TrainTimeDelegate(const DiagramOptions& ops, QObject* parent, TrainTime::TimeFormat format) :
    QStyledItemDelegate(parent), _ops(ops), _format(format)
{

}

QWidget* TrainTimeDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto* ed = new TrainTimeEdit(parent);
    connect(ed, SIGNAL(timeChanged(TrainTime)), this, SLOT(onTimeChanged()));
    setupEditor(ed);
    return ed;
}

void TrainTimeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    TrainTimeEdit* ed = static_cast<TrainTimeEdit*>(editor);
    //ed->setTime(index.data(Qt::EditRole).toTime());
	ed->setTime(qvariant_cast<TrainTime>(index.data(Qt::EditRole)));
}

void TrainTimeDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model,
    const QModelIndex& index) const
{
    auto* ed = static_cast<TrainTimeEdit*>(editor);
    QVariant v;
    v.setValue(ed->time());
    model->setData(index, v, Qt::EditRole);
}

QString TrainTimeDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    Q_UNUSED(locale);
    return qvariant_cast<TrainTime>(value).toString(_format);
}

void TrainTimeDelegate::setupEditor(TrainTimeEdit* ed) const
{
    ed->setFormat(_format);
	ed->setMaxHours(_ops.period_hours);
    //ed->setWrapping(true);  // <- This is actually useless for our TrainTimeEdit...
}

void TrainTimeDelegate::onTimeChanged()
{
    emit commitData(qobject_cast<QWidget*>(sender()));
}

TrainTimeQuickDelegate::TrainTimeQuickDelegate(const DiagramOptions& ops, QObject* parent, TrainTime::TimeFormat format_) :
    QStyledItemDelegate(parent), _ops(ops), _format(format_)
{
}

QWidget* TrainTimeQuickDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto* ed = new TrainTimeEdit(parent);
    connect(ed, SIGNAL(timeChanged(TrainTime)), this, SLOT(onTimeChanged()));
    setupEditor(ed);
    return ed;
}

void TrainTimeQuickDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    TrainTimeEdit* ed = static_cast<TrainTimeEdit*>(editor);
    ed->setTime(qvariant_cast<TrainTime>(index.data(qeutil::TimeDataRole)));
}

void TrainTimeQuickDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* ed = static_cast<TrainTimeEdit*>(editor);
    QVariant v;
    v.setValue(ed->time());
    model->setData(index, v, qeutil::TimeDataRole);
}

void TrainTimeQuickDelegate::setupEditor(TrainTimeEdit* ed) const
{
    ed->setFormat(_format);
	ed->setMaxHours(_ops.period_hours);
    ed->setWrapping(true);
}

void TrainTimeQuickDelegate::onTimeChanged()
{
    emit commitData(qobject_cast<QWidget*>(sender()));
}