#pragma once

#include <QComboBox>
#include <set>

class PredefTrainFilterCore;
class TrainCollection;
/**
 * @brief The TrainFilterCombo class
 * The selection of Predef-/Sys- TrainFilters.
 * Use static members, and update data when necessary.
 */
class TrainFilterCombo : public QComboBox
{
    Q_OBJECT
    TrainCollection& coll;
    static std::set<TrainFilterCombo*> instances;
    const PredefTrainFilterCore* _current=nullptr;
public:
    TrainFilterCombo(TrainCollection& coll, QWidget* parent=nullptr);
    ~TrainFilterCombo();
    const PredefTrainFilterCore* current(){return _current;}

    static void refreshAll();
private:

signals:

    /**
     * @brief selectionChanged
     * Mind, the argument may be null, in cases where non-predef item is selected.
     */
    void filterChanged(const PredefTrainFilterCore*);

private slots:
    void onIndexChanged(int id);
public slots:
    void refreshData();
};

