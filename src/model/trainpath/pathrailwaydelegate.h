#pragma once
#include <QStyledItemDelegate>

class RailCategory;
class PathModel;
class QComboBox;

class PathRailwayDelegate : public QStyledItemDelegate
{
    RailCategory& railcat;
    PathModel* pathModel;
public:
    PathRailwayDelegate(RailCategory& cat, PathModel* model, QObject* parent=nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private:
    void setComboUnconstrained(QComboBox* cb, const QString& cur_name)const;

    void setComboConstrained(QComboBox* cb, const QString& cur_name, int row)const;
};
