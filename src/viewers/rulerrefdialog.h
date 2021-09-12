#pragma once

#include <QDialog>
#include <QStandardItemModel>

class Ruler;
class TrainAdapter;
class Train;

/**
 * @brief The RulerRefModel class
 * 标尺对照。适用于单个车次，一次性展示一个Adapter的情况，可以切换。
 * 如果切换Adapter，则reset
 * 应保证所给车次在本运行图至少有一个Adapter，否则Dialog就不构造
 */
class RulerRefModel:
        public QStandardItemModel
{
    Q_OBJECT;
    std::shared_ptr<Train> train;
    std::shared_ptr<TrainAdapter> adp{};
    std::shared_ptr<Ruler> ruler{};
public:
    enum Columns{
        ColInterval=0,
        ColPass,    //标准通通时分
        ColStart,
        ColStop,
        ColStd,     //算了起停的标准时分
        ColReal,
        ColMile,
        ColSpeed,
        ColAppend,
        ColDiff,
        ColMAX
    };

    RulerRefModel(std::shared_ptr<Train> train_,
                  QObject* parent=nullptr);
    
public slots:
    /**
     * @brief setIndex
     * 选择下标为i的TrainAdapter，更新数据
     */
    void setRailwayIndex(int i);

    /**
     * 注意初始为0，即无标尺。下标0也是无标尺。
     */
    void setRulerIndex(int i);

private:
    /**
     * 如果当前保存了标尺，一并更新了。
     * 标尺更新时也应当调用这个，把数据一并更新，以尽可能保证安全性。
     */
    void setupModel();
};

class QComboBox;
class QTableView;

/**
 * @brief The RulerRefDialog class
 * 标尺对照
 */
class RulerRefDialog : public QDialog
{
    Q_OBJECT;
    std::shared_ptr<Train> train;
    RulerRefModel* const model;
    
    QComboBox* cbRail, * cbRuler;
    QTableView* table;
    bool updating = false;
public:
    RulerRefDialog(std::shared_ptr<Train> train_, QWidget* parent = nullptr);
private:
    void initUI();
private slots:
    void onRailIndexChanged(int i);
};


