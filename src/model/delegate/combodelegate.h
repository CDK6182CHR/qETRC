#pragma once

#include <QStyledItemDelegate>
#include <QList>
#include <QString>

class ComboDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    const QStringList texts;
public:
    explicit ComboDelegate(const QStringList& texts_,QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

};
