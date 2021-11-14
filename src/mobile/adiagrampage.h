#pragma once

#ifdef QETRC_MOBILE

#include <QWidget>

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

signals:

public slots:
    void refreshData();

};

#endif
