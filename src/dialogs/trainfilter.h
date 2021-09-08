#pragma once

#include <QDialog>
#include "util/buttongroup.hpp"
#include "data/train/train.h"
#include "data/train/trainfiltercore.h"

class Routing;
class QEControlledTable;
class QEMoveableModel;
class QListView;
class TrainType;
class TrainCollection;
class QTableView;
class QStandardItemModel;
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


class QCheckBox;
class Diagram;
/**
 * @brief The TrainFilter class
 * pyETRC.TrainFilter  功能类似，逻辑基本照搬，但直接继承QDialog
 */
class TrainFilter : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    TrainFilterCore core;

    QCheckBox* ckType,*ckInclude,*ckExclude,*ckRouting;
    QCheckBox* ckShowOnly,*ckInverse;
    RadioButtonGroup<3> *gpPassen;

    SelectTrainTypeDialog* dlgType=nullptr;
    TrainNameRegexDialog* dlgInclude=nullptr,*dlgExclude=nullptr;
    SelectRoutingDialog* dlgRouting=nullptr;

public:
    TrainFilter(Diagram& diagram_, QWidget* parent=nullptr);
    bool check(std::shared_ptr<const Train> train)const;
    const auto& getCore()const { return core; }

    /**
     * 返回当前筛选器状态下，被选中的车次列表。
     * 线性算法，暂时不引入缓存机制。
     */
    QList<std::shared_ptr<Train>> selectedTrains()const;

private:
    void initUI();
signals:
    void filterApplied(TrainFilter* s);
private slots:
    void selectType();
    void setInclude();
    void setExclude();
    void selectRouting();
    void actApply();
public slots:
    void clearFilter();
};

