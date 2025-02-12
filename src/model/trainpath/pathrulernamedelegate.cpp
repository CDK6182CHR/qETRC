#include "pathrulernamedelegate.h"

#include <QComboBox>

#include "data/trainpath/trainpath.h"
#include "data/rail/railway.h"
#include "data/rail/ruler.h"

PathRulerNameDelegate::PathRulerNameDelegate(const TrainPath* path, QObject* parent):
	QStyledItemDelegate(parent),
	m_path(path)
{
}

void PathRulerNameDelegate::setPath(const TrainPath* path)
{
	m_path = path;
}

QWidget* PathRulerNameDelegate::createEditor(QWidget* parent, [[maybe_unused]] const QStyleOptionViewItem& option,
	[[maybe_unused]] const QModelIndex& index) const
{
	auto* w = new QComboBox(parent);
	return w;
}

void PathRulerNameDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if (!index.isValid())
		return;
	auto* w = static_cast<QComboBox*>(editor);
	auto rail = m_path->segments().at(index.row()).railway.lock();
	w->clear();
	foreach(auto r, rail->rulers()) {
		w->addItem(r->name());
	}
	w->setCurrentIndex(index.data(Qt::EditRole).toInt());
}

void PathRulerNameDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	if (!index.isValid())
		return;
	auto* w = static_cast<QComboBox*>(editor);
	model->setData(index, w->currentIndex(), Qt::EditRole);
	model->setData(index, w->currentText(), Qt::DisplayRole);
}
