#pragma once

#ifdef QETRC_MOBILE

#include <QWidget>
#include <memory>

class Train;
class Diagram;
class DiagramWidget;
class QComboBox;
class QStackedWidget;


class ADiagramPage : public QWidget
{
    Q_OBJECT
    Diagram& diagram;
    QList<DiagramWidget*> pages;
    QComboBox* cbPages;
    QStackedWidget* stack;
public:
    explicit ADiagramPage(Diagram& diagram, QWidget *parent = nullptr);
private:
    void initUI();

    void clearPages();

    DiagramWidget* currentPage();
signals:
    void switchToTrain(std::shared_ptr<Train>);
public slots:
    void refreshData();
private:
    void actToTrain();

};

#endif
