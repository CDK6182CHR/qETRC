#pragma once

#ifdef QETRC_MOBILE

#include <QWidget>
#include <data/diagram/diagram.h>

class QLineEdit;
class AStartPage : public QWidget
{
    Q_OBJECT;
    Diagram diagram;
    QLineEdit* edDiaName;
public:
    explicit AStartPage(QWidget *parent = nullptr);
    auto& getDiagram(){return diagram;}
private:
    void initUI();


signals:
    void diagramRefreshed();


private slots:
    void actOpen();

    void actClear();

public slots:
    void refreshData();
};

#endif
