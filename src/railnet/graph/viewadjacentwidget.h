#pragma once

#include <QSplitter>

class AdjacentListWidget;
class VertexListWidget;
class RailNet;
/**
 * @brief The ViewAdjacentWidget class
 *  展示邻接表数据。暂定按照QWidget，但是给成Dialog的Flag
 */
class ViewAdjacentWidget : public QSplitter
{
    Q_OBJECT
    const RailNet& net;
    VertexListWidget* verWidget;
    AdjacentListWidget* adjWidget;
public:
    explicit ViewAdjacentWidget(const RailNet& net, QWidget *parent = nullptr);
private:
    void initUI();
signals:

};

