#pragma once

#ifdef QETRC_MOBILE
#include <QWidget>
#include <memory>

class ARailAnalysis;
class SelectRailwayCombo;

class Railway;
class Diagram;
class QComboBox;
class QTabWidget;
class RailStationWidget;
class ForbidTabWidget;

/**
 * @brief The ARailPage class
 * 移动端的线路页面。控件只有一套，通过切换数据。
 */
class ARailPage : public QWidget
{
    Q_OBJECT
    Diagram& diagram;

    SelectRailwayCombo* cbRails;
    QTabWidget* tab;
    RailStationWidget* railWidget;
    ForbidTabWidget* forbidWidget;
    ARailAnalysis* anaWidget;
public:
    explicit ARailPage(Diagram& diagram, QWidget *parent = nullptr);
private:
    void initUI();

signals:

public slots:
    void setRailway(std::shared_ptr<Railway> railway);
    void refreshData();
};

#endif // ARAILPAGE_H
