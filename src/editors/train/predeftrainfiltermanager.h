#pragma once

#include <QSplitter>

class PredefTrainFilterCore;
class PredefTrainFilterWidget;
class PredefTrainFilterList;
class TrainCollection;
class PredefTrainFilterManager : public QSplitter
{
    Q_OBJECT
    TrainCollection& coll;
    PredefTrainFilterList* lstWidget;
    PredefTrainFilterWidget* editWidget;
public:
    explicit PredefTrainFilterManager(TrainCollection& coll,QWidget *parent = nullptr);
    auto* list(){return lstWidget;}
    auto* editor(){return editWidget;}
private:
    void initUI();
signals:
    void addFilter(TrainCollection& coll);
    void removeFilter(TrainCollection& coll, int id);
    void updateFilter(PredefTrainFilterCore* filter, std::unique_ptr<PredefTrainFilterCore>& data);
public slots:
    void commitAddFilter(int place, const PredefTrainFilterCore* filter);
    void commitRemoveFilter(int place, const PredefTrainFilterCore* filter);
    void commitUpdateFilter(const PredefTrainFilterCore* filter);
    void refreshData();
    void refreshTagCompleter();
};

