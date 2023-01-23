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
    const TrainFilterCore* filter(){return _current;}
private:
    void initUI();

signals:
    void filterChanged(const TrainFilterCore*);

private slots:
    void onComboChanged(const PredefTrainFilterCore* core);
    void onDialogApplied();
};

