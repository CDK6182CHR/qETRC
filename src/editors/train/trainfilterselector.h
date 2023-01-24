#pragma once

#include <QWidget>
#include "data/train/trainfilterselectorcore.h"

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
    TrainFilterSelectorCore selector;
public:
    explicit TrainFilterSelector(TrainCollection& coll, QWidget *parent = nullptr);

    /**
    * This version also initializes dialog->core with given data
    */
    explicit TrainFilterSelector(TrainCollection& coll, const TrainFilterCore& initData, QWidget *parent = nullptr);
    const TrainFilterCore* filter(){return selector.filter();}
    const TrainFilterSelectorCore& core(){return selector;}
    auto* dialog(){return dlg;}
private:
    void initUI();

signals:
    void filterChanged(const TrainFilterCore*);

private slots:
    void onComboChanged(const PredefTrainFilterCore* core);
    void onDialogApplied();
};

