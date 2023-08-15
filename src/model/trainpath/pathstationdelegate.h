#pragma once

#include <QStyledItemDelegate>

class PathModel;
class PathStationDelegate : public QStyledItemDelegate
{
    PathModel* pathModel;
public:
    explicit PathStationDelegate(PathModel* model, QObject *parent = nullptr);

    // QAbstractItemDelegate interface
public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

