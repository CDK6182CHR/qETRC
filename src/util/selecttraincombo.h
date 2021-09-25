
#pragma once

#include <QHBoxLayout>
#include <QList>
#include <memory>

class TrainCollection;
class QComboBox;
class QLineEdit;
class Train;

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
    SelectTrainCombo(TrainCollection& coll_, std::shared_ptr<Train> train,
                     QWidget* parent=nullptr);
    auto train(){return _train;}

    /**
     * 弹出对话框，选择列车。
     * 如果取消了，返回空
     */
    static std::shared_ptr<Train> dialogGetTrain(TrainCollection& coll_, QWidget* parent,
        const QString& title = tr("选择车次"), const QString& prompt = "");

    /**
     * @brief setTrain
     * 从外部设置列车。要同时更新参数以及设置Combo的数据。
     * 同时发射信号。
     */
    void setTrain(std::shared_ptr<Train> train);

    /**
     * 清空输入，不发射信号
     */
    void resetTrain();

private :
    void initUI();
    void addTrainItem(std::shared_ptr<Train> t);

    /**
     * @brief updateCombo
     * 根据match的数据，更新combo
     */
    void updateCombo();
signals:
    void currentTrainChanged(std::shared_ptr<Train>);
private slots:
    void onEditingFinished();
    void onIndexChanged(int i);
public slots:
    void setEnabled(bool on);
    void focusIn();
};

