#pragma once

#include <QTreeView>
#include <QMenu>
#include <QPoint>

#include "data/diagram/diagrampage.h"
#include "model/diagram/diagramnavimodel.h"

/**
 * @brief The NaviTree class  系统导航那个TreeView
 */
class NaviTree : public QTreeView
{
    Q_OBJECT

    QMenu* mePageList;
    DiagramNaviModel* _model;
public:
    NaviTree(DiagramNaviModel* model_,QWidget* parent=nullptr);

protected:
    virtual void currentChanged(const QModelIndex& cur, const QModelIndex& prev)override;

private:
    void initContextMenus();
    using ACI = navi::AbstractComponentItem;
    ACI* getItem(const QModelIndex& idx);


signals:
    void pageAdded(std::shared_ptr<DiagramPage> page);
    void focusInPage(std::shared_ptr<DiagramPage> page);
    void focusOutPage();
    void focusInTrain(std::shared_ptr<Train> train);
    void focusOutTrain();

private slots:
    void showContextMenu(const QPoint& pos);
    void addNewPage();
    
};


