#pragma once
#include <QWidget>

class QTableWidget;
class Railway;

class RailTopoTable : public QWidget
{
    Q_OBJECT
    std::shared_ptr<Railway> railway;

    /**
     * @brief table
     * 这里用QTableWidget算是个特例。原因是需要大量搞合并单元格的操作，
     * Model/View强绑定。
     */
    QTableWidget* table;
public:
    enum {
        ColCounter,
        ColMile,
        ColName,
        ColDown,
        ColUp,
        ColInfo,
        ColMAX
    };
    explicit RailTopoTable(std::shared_ptr<Railway> rail,
                           QWidget *parent = nullptr);
    void refreshData();

private:
    void initUI();

signals:

};

