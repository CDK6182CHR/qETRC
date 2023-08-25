#pragma once

#include <set>
#include <QDialog>

class QTableView;
class TrainListReadModel;
class TrainCollection;
class TrainPath;
/**
 * @brief The PathTrainsDialog class
 * 2023.08.24  The dialog for viewing and removing trains that belong to a path.
 * This dialog is designed to show in modal mode, to avoid complicate status maintaining.
 */
class PathTrainsDialog : public QDialog
{
    Q_OBJECT
    TrainCollection& coll;
    TrainPath* path;

    TrainListReadModel* model;
    QTableView* table;
public:
    PathTrainsDialog(TrainCollection& coll, TrainPath* path, QWidget* parent=nullptr);
    void refreshData();

private:
    void initUI();

signals:
    void actAdd();

    // the order of std::set is required
    void removeTrains(TrainPath*, const std::set<int>& rows);

private slots:
    void actRemove();
};

