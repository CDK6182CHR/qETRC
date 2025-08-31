#pragma once
#include <memory>
#include <QDialog>

class QTableView;
class TrainListReadModel;
class TrainPath;
class TrainCollection;
class Train;
struct DiagramOptions;
class TrainNameSortProxyModel;

/**
 * @brief The AddTrainsToPathDialog class
 * 2023.08.26  Add trains to a single path.
 * This dialog is typically used in modal state, to avoid complicate status maintainance.
 */
class AddTrainsToPathDialog : public QDialog
{
    Q_OBJECT;
    const DiagramOptions& _ops;
    TrainCollection& coll;
    TrainPath* path;

    TrainListReadModel* model;
    TrainNameSortProxyModel* pmodel;
    QTableView* table;

public:
    AddTrainsToPathDialog(const DiagramOptions& ops, TrainCollection& coll, TrainPath* path, QWidget* parent=nullptr);
    void refreshData();

signals:
    void trainsAdded(TrainPath*, QList<std::shared_ptr<Train>>);

private:
    void initUI();

    // QDialog interface
public slots:
    virtual void accept() override;
};

