
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

    /**
     * 弹出对话框，选择列车。
     * 如果取消了，返回空
     */
    static std::shared_ptr<Train> dialogGetTrain(TrainCollection& coll_, QWidget* parent,
        const QString& title = tr("选择车次"), const QString& prompt = "");

private :
    void initUI();
signals:
    void currentTrainChanged(std::shared_ptr<Train>);
private slots:
    void onEditingFinished();
    void onIndexChanged(int i);
};

