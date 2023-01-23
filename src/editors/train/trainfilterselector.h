#pragma once

#include <QWidget>

class PredefTrainFilterCore;
class TrainFilterCore;
class TrainCollection;
class TrainFilterCombo;
class TrainFilterDialog;

class TrainFilterSelector : public QWidget
{
    Q_OBJECT
    TrainCollection& coll;
    TrainFilterDialog* dlg;
    TrainFilterCombo* combo;
    const TrainFilterCore* _current;
public:
    explicit TrainFilterSelector(TrainCollection& coll, QWidget *parent = nullptr);

    /**
    * This version also initializes dialog->core with given data
    */
    explicit TrainFilterSelector(TrainCollection& coll, const TrainFilterCore& initData, QWidget *parent = nullptr);
    const TrainFilterCore* filter(){return _current;}
    auto* dialog(){return dlg;}
private:
    void initUI();

signals:
    void filterChanged(const TrainFilterCore*);

private slots:
    void onComboChanged(const PredefTrainFilterCore* core);
    void onDialogApplied();
};

