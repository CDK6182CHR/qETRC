#pragma once

#include <QStyledItemDelegate>

class PostiveSpinDelegate : public QStyledItemDelegate
{
    int step;
public:
    explicit PostiveSpinDelegate(int step_, QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
};

