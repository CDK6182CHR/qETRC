#pragma once

#include <QWidget>

#include <memory>

class Railway;
class RulerModel;
class Ruler;

class QTableView;
class QLineEdit;
/**
 * @brief The RulerWidget class
 * 标尺编辑的面板。与pyETRC的实现比较类似，但不做那么多按钮。
 */
class RulerWidget : public QWidget
{
    Q_OBJECT;
    std::shared_ptr<Ruler> ruler;
    RulerModel* model;

    QLineEdit* edName;
    QTableView* table;
    const bool commitInPlace;
    bool updating = false;
public:
    explicit RulerWidget(std::shared_ptr<Ruler> ruler_, bool commitInPlace,
                         QWidget *parent = nullptr);
    void refreshData();
    void refreshBasicData();
    //void setRuler(std::shared_ptr<Ruler> ruler);
    auto getRuler(){return ruler;}
    auto getModel(){return model;}

    virtual bool event(QEvent* e)override;

private:
    void initUI();

private slots:
    void actApply();
    void actCancel();
    void onDiffChanged(bool on);

    void copyFromDownToUp();   //主要是询问一下  操作在model里面
    void copyFromUpToDown();
    void actRemove();

signals:
    void actChangeRulerData(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> nr);
    void actChangeRulerName(std::shared_ptr<Ruler> ruler, const QString& name);
    void focusInRuler(std::shared_ptr<Ruler>ruler);
    void actRemoveRuler(std::shared_ptr<Ruler>);

};

