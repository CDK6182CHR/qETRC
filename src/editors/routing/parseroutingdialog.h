#pragma once

#include <QDialog>
#include <memory>

class QTextBrowser;
class QTextEdit;
class QCheckBox;
class QLineEdit;
class Routing;
class TrainCollection;
/**
 * @brief The ParseRoutingDialog class
 * 解析单个车次文本字符串。
 * 如果从Editor调起，则添加到表格中；如果从context调起，则直接应用更改。
 * 但在内部都是没有区别的，只有一个提示不一样
 */
class ParseRoutingDialog : public QDialog
{
    Q_OBJECT;
    TrainCollection& coll;
    bool fromContext;
    std::shared_ptr<Routing> origin;
    QLineEdit* edSplitter;
    QCheckBox* ckFullOnly;
    QTextEdit* edText;
    QTextBrowser* edOutput;
public:
    ParseRoutingDialog(TrainCollection& coll_, bool fromContext_,
                       std::shared_ptr<Routing> original,
                       QWidget* parent=nullptr);
signals:
    void routingParsed(std::shared_ptr<Routing> origin, std::shared_ptr<Routing> tmp);
private:
    void initUI();
private slots:
    void actApply();
};

