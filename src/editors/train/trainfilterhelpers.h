#pragma once

#include <QDialog>
#include <QSet>
#include <memory>

class Routing;
class QTableView;
class QEControlledTable;
class QEMoveableModel;
class TrainType;
class QListView;
class QStandardItemModel;
class TrainCollection;

/**
 * @brief The SelectTrainTypeDialog class
 * 选择列车种类。给出本运行图中所有有对应车次的类型。
 */
class SelectTrainTypeDialog: public QDialog
{
    Q_OBJECT;
    TrainCollection& coll;
    QStandardItemModel* model;
    QListView* view;
    QSet<std::shared_ptr<const TrainType>> _selected;
public:
    SelectTrainTypeDialog(TrainCollection& coll_, QWidget* parent=nullptr);
    const auto& selected()const {return _selected;}
private:
    void initUI();

private slots:
    void actApply();
    void refreshTypes();
public slots:
    void showDialog();
    void clearSelected();
};

/**
 * @brief The TrainNameRegexDialog class
 * 设置列车正则表达式的Dialog；包含、排除车次都用这个。
 */
class TrainNameRegexDialog: public QDialog
{
    Q_OBJECT
    TrainCollection& coll;
    QEMoveableModel* model;
    QEControlledTable* ctab;
    QTableView* table;
    QVector<QRegExp> _names;
public:
    TrainNameRegexDialog(TrainCollection& coll_, QWidget* parent=nullptr);
    auto& names()const {return _names;}
private:
    void initUI();
private slots:
    void actApply();
    void refreshData();
public slots:
    void clearNames();
};


/**
 * @brief The SelectRoutingDialog class
 * 选择交路  注意第0行是空交路
 */
class SelectRoutingDialog: public QDialog
{
    Q_OBJECT;
    TrainCollection& coll;
    QStandardItemModel* model;
    QListView* view;
    QSet<std::shared_ptr<const Routing>> _selected;
    bool _containsNull=false;
public:
    SelectRoutingDialog(TrainCollection& coll_, QWidget* parent=nullptr);
    auto& selected()const{return _selected;}
    bool containsNull()const{return _containsNull;}
private:
    void initUI();
private slots:
    void actApply();
    void refreshRoutings();
public slots:
    void showDialog();
    void clearSelection();
};

