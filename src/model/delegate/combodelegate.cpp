#include "combodelegate.h"
#include <QtWidgets>

ComboDelegate::ComboDelegate(const QStringList& texts_, QObject* parent):
	QStyledItemDelegate(parent),texts(texts_)
{
}

QWidget* ComboDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option)
	if (!index.isValid())
		return nullptr;
	auto* combo = new QComboBox(parent);
	combo->addItems(texts);
	return combo;
}

void ComboDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if (!index.isValid())
		return;
	auto* combo = static_cast<QComboBox*>(editor);
	combo->setCurrentIndex(qvariant_cast<int>(index.data(Qt::EditRole)));
}

void ComboDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	if (!index.isValid())return;
	auto* combo = static_cast<QComboBox*>(editor);
	model->setData(index, combo->currentIndex(), Qt::EditRole);
}

QString ComboDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
	bool ok = false;
	const int index = value.toInt(&ok);
	if (ok && index >= 0 && index < texts.size())
		return texts.at(index);
	return QStyledItemDelegate::displayText(value, locale);
}
