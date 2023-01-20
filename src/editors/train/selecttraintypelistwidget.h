#pragma once

#include <QListView>
#include <memory>

class TrainFilterCore;
class TrainType;
class QStandardItemModel;
class TrainCollection;
/**
 * @brief The SelectTrainTypeListWidget class
 * 2023.01.20  add  previous SelectTrainTypeDialog,
 * this version works with new TrainFilterBasicWidget
 */
class SelectTrainTypeListWidget : public QListView
{
    Q_OBJECT;
    TrainCollection& coll;
    QStandardItemModel* model;
public:
    SelectTrainTypeListWidget(TrainCollection& coll_, QWidget* parent=nullptr);
    QSet<std::shared_ptr<const TrainType>> selected()const;
private:
    void initUI();

public slots:
    void refreshTypes();
    void refreshTypesWithSelection(const QSet<std::shared_ptr<const TrainType>>& selected);
    void clearSelected();
};

