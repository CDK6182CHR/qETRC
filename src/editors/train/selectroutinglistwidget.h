#pragma once
#include <QListView>
#include <utility>

class Routing;
class QStandardItemModel;
class TrainCollection;
class SelectRoutingListWidget : public QListView
{
    Q_OBJECT
    TrainCollection& coll;
    QStandardItemModel* model;

    /**
     * Selected routings, and contains null
     */
    using res_t = std::pair<QSet<std::shared_ptr<const Routing>>,bool>;
    res_t _selected;
public:
    SelectRoutingListWidget(TrainCollection& coll_, QWidget* parent=nullptr);
    res_t result()const;
    void refreshRoutings();
    void refreshRoutingsWithSelection(const res_t& _selected);
private:
    void initUI();
public slots:

    void clearSelection();
};

