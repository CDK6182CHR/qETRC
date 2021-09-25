#pragma once

#include <QStandardItemModel>
#include <QWidget>

#include "railnet.h"

/**
 * @brief The VertexListModel class
 * 图结点列表Model。站名那一列中保存const的结点指针。
 */
class VertexListModel: public QStandardItemModel
{
    Q_OBJECT
    const RailNet& net;
public:
    enum {
        ColName=0,
        ColInDeg,
        ColOutDeg,
        ColLevel,
        ColPassenger,
        ColFreight,
        ColMAX
    };

    explicit VertexListModel(const RailNet& net, QObject* parent=nullptr);
    void refreshData();
    std::shared_ptr<RailNet::vertex> vertexForRow(int row)const;

    /**
     * 搜索全车站，返回符合条件的；或-1表示无符合条件的。
     */
    int searchStation(const QString& name);
private:
    void setupModel();
};

class QLineEdit;
class QTableView;

/**
 * @brief The VertexListWidget class
 * 图结点列表，用于展示直接的邻接表数据。
 */
class VertexListWidget : public QWidget
{
    Q_OBJECT
    VertexListModel* const model;

    QLineEdit* edSearch;
    QTableView* table;
public:
    explicit VertexListWidget(const RailNet& net, QWidget *parent = nullptr);
    void refreshData();
    auto* getModel(){return model;}
private:
    void initUI();
signals:
    void currentVertexChanged(std::shared_ptr<RailNet::vertex>);
private slots:
    void searchStation();
    void onRowChanged(const QModelIndex& idx);
};

