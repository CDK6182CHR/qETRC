#pragma once

#include <QWidget>

class PredefTrainFilterCore;
class QStandardItemModel;
class QListView;
class TrainCollection;

class PredefTrainFilterList : public QWidget
{
    Q_OBJECT
    TrainCollection& coll;
    QListView* view;
    QStandardItemModel* model;

public:
    explicit PredefTrainFilterList(TrainCollection& coll, QWidget *parent = nullptr);

private:
    void initUI();

signals:
    void currentChanged(PredefTrainFilterCore* core);

    /**
     * @brief addFilter
     * For memory safety consideration, the object is not created here,
     * but created in the qecmd class.
     */
    void addFilter(TrainCollection& coll);

    void removeFilter(TrainCollection& coll, int id);

private slots:
    void listSelectionChanged(const QModelIndex& idx);
    void actAdd();
    void actRemove();
public slots:
    void refreshList();
};

