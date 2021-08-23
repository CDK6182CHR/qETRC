#pragma once

#include <QWidget>
#include <QTabWidget>
#include <memory>

#include "model/rail/forbidmodel.h"
#include "data/rail/forbid.h"

class QTableView;
class QCheckBox;
class QSpinBox;
class QMenu;

/**
 * @brief The ForbidWidget class
 * 天窗编辑的一个页面，也就是一个Tab
 * 显示与否的变化，搞到Context去
 */
class ForbidWidget : public QWidget
{
    Q_OBJECT;
    const std::shared_ptr<Forbid> forbid;
    ForbidModel*const model;
    QCheckBox* ckDown,*ckUp;
    QSpinBox* spLength;
    QTableView* table;
    QMenu* context;
    bool inplace;

    bool updating=false;

public:
    explicit ForbidWidget(std::shared_ptr<Forbid> forbid, bool commitInPlace, QWidget *parent = nullptr);
    auto* getModel(){return model;}
    auto getForbid(){return forbid;}

signals:
    void forbidChanged(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> data);

    /**
     * @brief forbidShowToggled
     * 指定方向的天窗是否显示变化。发射前确保是变了的；cmd只管反转
     */
    void forbidShowToggled(std::shared_ptr<Forbid> forbid, Direction dir);

public slots:
    void refreshBasicData();
    void refreshData();

private:
    void initUI();
    void initContextMenu();
    int currentRow();

private slots:
    void onApply();
    void onDownShowToggled(bool on);
    void onUpShowToggled(bool on);

    // context menu actions:
    void copyToNextRow();
    void copyToAllRows();
    void calculateBegin();
    void calculateEnd();
    void calculateAllBegin();
    void calculateAllEnd();

    void onContextMenu(const QPoint& pos);
    

};



class ForbidTabWidget:
        public QTabWidget
{
    Q_OBJECT;
    const std::shared_ptr<Railway> railway;
    bool inplace;
public:
    ForbidTabWidget(std::shared_ptr<Railway> railway,bool commitInPlace, QWidget* parent=nullptr);
    auto getRailway(){return railway;}
private:
    void addForbidTab(std::shared_ptr<Forbid> forbid);
signals:
    void forbidChanged(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> data);
    /**
     * @brief forbidShowToggled
     * 指定方向的天窗是否显示变化。发射前确保是变了的；cmd只管反转
     */
    void forbidShowToggled(std::shared_ptr<Forbid> forbid, Direction dir);
public slots:
    void refreshData(std::shared_ptr<Forbid> forbid);
    void refreshBasicData(std::shared_ptr<Forbid> forbid);
    /**
     * 将操作转发给下属的每个页面的Model
     */
    void updateAllRailIntervals(std::shared_ptr<Railway> railway, bool equiv);

};

