#pragma once

#include <QDialog>

class QTextBrowser;
class QCheckBox;
class Routing;
class TrainCollection;
/**
 * @brief The DetectRoutingDialog class
 * 新增：交路车次识别的Dialog。
 * original: 直接传入原始的就行
 */
class DetectRoutingDialog : public QDialog
{
    Q_OBJECT;
    TrainCollection& coll;
    std::shared_ptr<Routing> origin;
    const bool fromContext;
    QCheckBox* ckFullOnly;
    QTextBrowser* edOutput;
public:
    DetectRoutingDialog(TrainCollection& coll_, std::shared_ptr<Routing> original,
                        bool fromContext_, QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void routingDetected(std::shared_ptr<Routing> origin, std::shared_ptr<Routing> res);
private slots:
    /**
     * @brief onApply
     * 先复制交路，然后进行识别操作，显示结果报告，然后发送信号。
     */
    void onApply();
};

class BatchDetectRoutingDialog :public QDialog
{
    Q_OBJECT;
    TrainCollection& coll;
    QCheckBox* ckFull;
    QTextBrowser* edOutput;
public:
    BatchDetectRoutingDialog(TrainCollection& coll, QWidget* parent = nullptr);
private:
    void initUI();
private slots:
    void actApply();
signals:

    /**
     * 同时记录Routing的下标。
     */
    void detectApplied(const QVector<int>& indexes,
        const QVector<std::shared_ptr<Routing>>& data);
};

