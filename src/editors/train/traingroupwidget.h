#pragma once

#if 0
#include <QWidget>
#include <memory>

class QLineEdit;
class TrainCollection;
class TrainGroup;

/**
 * @brief The TrainGroupWidget class
 * Configuration of TrainGroup. The data field is nullable.
 * NOT UNSED FOR CURRENT VERSION
 */
class TrainGroupWidget : public QWidget
{
    Q_OBJECT

    TrainCollection& coll;
    /**
     * @brief _group  This field may be null.
     */
    std::shared_ptr<TrainGroup> _group;

    QLineEdit* edName, *edNote;
public:
    explicit TrainGroupWidget(TrainCollection& coll, QWidget *parent = nullptr);
    void setData(std::shared_ptr<TrainGroup> group);
private:
    void initUI();

signals:

public slots:
    void refreshData();

};
#endif
