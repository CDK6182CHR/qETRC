#pragma once

#include <set>
#include <memory>
#include <QDialog>

class QTableView;
class TrainListReadModel;
class TrainCollection;
class TrainTag;
struct DiagramOptions;
class Train;

/**
 * @brief  2025.08.14  Dialog displaying trains with the given tag.
 * Mainly copied from PathTrainsDialog.
 */
class TagTrainsDialog : public QDialog
{
    Q_OBJECT;
    const DiagramOptions& m_ops;
    TrainCollection& m_coll;
    std::shared_ptr<TrainTag> m_tag;

    TrainListReadModel* m_model;
    QTableView* m_table;
public:
    TagTrainsDialog(const DiagramOptions& ops, TrainCollection& coll, std::shared_ptr<TrainTag> tag, QWidget* parent = nullptr);

private:
    void initUI();

signals:
    void actAdd();

    // the order of std::set is required

    void removeTrains(std::shared_ptr<TrainTag> tag, const std::vector<std::pair<std::shared_ptr<Train>, int>>& trains);

private slots:
    void actRemove();

public slots:
    void refreshData();
};

