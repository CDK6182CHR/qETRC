#include "textcompletiondelegate.h"

#include <QLineEdit>
#include <QCompleter>
#include "model/train/traintaglistdirectmodel.h"

TextCompletionDelegate::TextCompletionDelegate(QCompleter* completer, QObject* parent):
	QStyledItemDelegate(parent), m_completer(completer)
{
}

QWidget* TextCompletionDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);
	auto* ed = new QLineEdit(parent);
	ed->setCompleter(m_completer);
	return ed;
}

void TextCompletionDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto* ed = qobject_cast<QLineEdit*>(editor);
	ed->setText(index.data(Qt::EditRole).toString());
}

void TextCompletionDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto* ed = qobject_cast<QLineEdit*>(editor);
	model->setData(index, ed->text(), Qt::EditRole);
}

QString TextCompletionDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
	Q_UNUSED(locale);
	return value.toString();
}

