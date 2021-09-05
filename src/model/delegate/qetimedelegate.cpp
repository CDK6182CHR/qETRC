#include "qetimedelegate.h"

#include <QTimeEdit>
#include "qedelegate.h"

QETimeDelegate::QETimeDelegate(QObject *parent, const QString &format):
    QStyledItemDelegate(parent),_format(format)
{

}

QWidget *QETimeDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
     auto* ed=new QTimeEdit(parent);
     connect(ed, SIGNAL(timeChanged(QTime)), this, SLOT(onTimeChanged()));
     setupEditor(ed);
     return ed;
}

void QETimeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QTimeEdit* ed=static_cast<QTimeEdit*>(editor);
    ed->setTime(index.data(Qt::EditRole).toTime());
}

void QETimeDelegate::setModelData(QWidget *editor,
                                  QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    auto* ed=static_cast<QTimeEdit*>(editor);
    model->setData(index,ed->time(),Qt::EditRole);
}

QString QETimeDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    Q_UNUSED(locale);
    return value.toTime().toString(_format);
}

void QETimeDelegate::setupEditor(QTimeEdit *ed) const
{
    ed->setDisplayFormat(_format);
    ed->setWrapping(true);
}

void QETimeDelegate::onTimeChanged()
{
    emit commitData(qobject_cast<QWidget*>(sender()));
}

TimeQuickDelegate::TimeQuickDelegate(QObject* parent, const QString& format_):
    QStyledItemDelegate(parent),_format(format_)
{
}

QWidget* TimeQuickDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto* ed = new QTimeEdit(parent);
    connect(ed, SIGNAL(timeChanged(QTime)), this, SLOT(onTimeChanged()));
    setupEditor(ed);
    return ed;
}

void TimeQuickDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QTimeEdit* ed = static_cast<QTimeEdit*>(editor);
    ed->setTime(index.data(qeutil::TimeDataRole).toTime());
}

void TimeQuickDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* ed = static_cast<QTimeEdit*>(editor);
    model->setData(index, ed->time(), qeutil::TimeDataRole);
}

void TimeQuickDelegate::setupEditor(QTimeEdit* ed) const
{
    ed->setDisplayFormat(_format);
    ed->setWrapping(true);
}

void TimeQuickDelegate::onTimeChanged()
{
    emit commitData(qobject_cast<QWidget*>(sender()));
}