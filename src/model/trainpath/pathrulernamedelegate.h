#pragma once

#include <QStyledItemDelegate>

class PathRuler;
class TrainPath;
class QComboBox;
class Railway;

/**
 * Delegate for the combo boxes of the PathRuler editor in the RulerName column.
 * This is not a trivial combo box delegate class, because we have different choices for different rows.
 */
class PathRulerNameDelegate : public QStyledItemDelegate
{
	const TrainPath* m_path;
	
public:
	PathRulerNameDelegate(const TrainPath* path, QObject* parent = nullptr);
	void setPath(const TrainPath* path);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    /**
     * For current impl, we choose to set the option list for each time the editor is called.
     * This may be slow, but we do not know (in detail) about the strategy of the delegation framework.
     */
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    //QString displayText(const QVariant &value, const QLocale &locale) const override;
};
