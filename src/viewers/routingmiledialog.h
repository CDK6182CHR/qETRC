#pragma once

#include <QStandardItemModel>
#include <QDialog>
#include <memory>

class Routing;
class QTableView;
struct DiagramOptions;

/**
 * @brief The RoutingMileModel class is provided for showing the mile
 * information of a routing object. This is a read-only model that initialized only once
 * and (typically) never updated later.
 * This is designed separately from the RoutingEditModel, since maintaining the mile
 * information can be complex when the train order changes.
 */
class RoutingMileModel: public QStandardItemModel
{
    Q_OBJECT;
    const DiagramOptions& m_ops;
    std::shared_ptr<Routing> m_routing;
public:
    enum Cols {
        ColTrainName = 0,
        ColVirtual,
        ColStarting,
        ColTerminal,
        ColLink,
        ColMile,
        ColAccMile,
        ColMAX
    };
    RoutingMileModel(const DiagramOptions& ops, std::shared_ptr<Routing> routing, QObject* parent=nullptr);
    void setupModel();
};

class RoutingMileDialog : public QDialog
{
    Q_OBJECT;
    const DiagramOptions& m_ops;
    std::shared_ptr<Routing> m_routing;
    RoutingMileModel* m_model;
    QTableView* m_table;
public:
    RoutingMileDialog(const DiagramOptions& ops, std::shared_ptr<Routing> routing, QWidget* parent=nullptr);
private:
    void initUI();
};

