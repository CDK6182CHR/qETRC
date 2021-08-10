#pragma once

#include <QHBoxLayout>
#include <QList>

#include "data/train/traincollection.h"

class QComboBox;
class QLineEdit;

/**
 * @brief The SelectTrainCombo class
 * 输入搜索车次内容，结束编辑后在Combo列出候选列表，
 * pyETRC中添加交路车次的组件。
 */
class SelectTrainCombo : public QHBoxLayout
{
    Q_OBJECT;
    TrainCollection& coll;
    std::shared_ptr<Train> _train{};
    QList<std::shared_ptr<Train>> matched{};

    QLineEdit* edName;
    QComboBox* cbTrains;
public:
    SelectTrainCombo(TrainCollection& coll_, QWidget* parent=nullptr);
    auto train(){return _train;}
private :
    void initUI();
signals:
    void currentTrainChanged(std::shared_ptr<Train>);
private slots:
    void onEditingFinished();
    void onIndexChanged(int i);
};

