#pragma once
#include <QDialog>
#include <memory>

class QCheckBox;
class QLineEdit;
class QRadioButton;
class Train;
class TrainCollection;
class SelectTrainCombo;

/**
 * @brief The AddRoutingNodeDialog class
 * 添加交路结点的对话框
 * 与pyETRC的区别是：取消Tab的设计，都在一个页面；
 * 添加车次时不进行检查，最后提交时统一检查有效性。
 * 考虑到可能会经常用到，不设置DeleteOnClose
 */
class AddRoutingNodeDialog : public QDialog
{
    Q_OBJECT;
    TrainCollection& coll;
    int row=0;
    SelectTrainCombo* cbTrain;
    QRadioButton* rdVirtual;
    QLineEdit* edName,*edStarting,*edTerminal;
    QCheckBox* ckLink;
public:
    AddRoutingNodeDialog(TrainCollection& coll_, QWidget* parent=nullptr);
signals:
    void realTrainAdded(int row, std::shared_ptr<Train> train, bool link);
    void virtualTrainAdded(int row, const QString& name, bool link);
private:
    void initUI();
private slots:
    void virtualToggled(bool on);
    void onTrainChanged(std::shared_ptr<Train> train);
    void actApply();
    void clearPage();
public slots:
    void openForRow(int row);
};

