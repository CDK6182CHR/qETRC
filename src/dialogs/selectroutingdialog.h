#pragma once
#include <QDialog>
#include <memory>

class Routing;
class TrainCollection;
class RoutingCollectionModel;
class QTableView;
/**
 * 2024.03.25 dialog for select routing, optionally supports add new routing.
 */
class SelectRoutingDialog: public QDialog {
    Q_OBJECT

    TrainCollection& coll;
    bool allowNew;
    RoutingCollectionModel* const model;

    QTableView* table;

    /**
     * Set this flag if NEW routing is created.
     */
    bool isNewSelected=false;

public:
    SelectRoutingDialog(TrainCollection& coll, bool allowNew_, QWidget* parent=nullptr);

    struct SelectReturned {
        bool isAccepted=true;
        bool createNew=false;
        std::shared_ptr<Routing> routing={};
    };

    static SelectReturned selectRouting(TrainCollection& coll, bool allowNew,
                                        QWidget* parent=nullptr);

private:
    void initUI();
    std::shared_ptr<Routing> currentRouting();

signals:
    void createNewRouting();
    void routingSelected(std::shared_ptr<Routing>);

private slots:
    void actApply();
    void actNew();
};
