#pragma once

#include <QStyledItemDelegate>

class QCompleter;

/**
 * Text completion delegate: simple line edit delegate with completer specified.
 * Mainly used for train tag.
 */
class TextCompletionDelegate: public QStyledItemDelegate
{
	Q_OBJECT;
    QCompleter* m_completer;
public:
    TextCompletionDelegate(QCompleter* completer, QObject* parent = nullptr);

    virtual QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    virtual void setModelData(QWidget* editor,
        QAbstractItemModel* model, const QModelIndex& index) const override;
    virtual QString displayText(const QVariant& value,
        const QLocale& locale) const override;

};

