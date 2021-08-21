#pragma once

#include <QWidget>
#include <QTabWidget>
#include <memory>

#include "model/rail/forbidmodel.h"

class QTableView;
class Forbid;
class QCheckBox;
class QSpinBox;

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
    bool inplace;

    bool updating=false;

public:
    explicit ForbidWidget(std::shared_ptr<Forbid> forbid, bool commitInPlace, QWidget *parent = nullptr);
    auto* getModel(){return model;}
    auto getForbid(){return forbid;}

signals:
    void forbidChanged(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> data);

public slots:
    void refreshBasicData();
    void refreshData();

private:
    void initUI();

private slots:
    void onApply();

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
public slots:
    void refreshData(std::shared_ptr<Forbid> forbid);
    void refreshBasicData(std::shared_ptr<Forbid> forbid);
};

