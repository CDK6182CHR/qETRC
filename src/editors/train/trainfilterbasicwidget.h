#pragma once

#include <QWidget>

/**
 * @brief The TrainFilterBasic class
 * 2023.01.15 The part for editing TrainFilter, both predefined and temporary.
 */
class TrainFilterBasicWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrainFilterBasicWidget(QWidget *parent = nullptr);

private:
    void initUI();
private slots:
    void selectType();
    void setInclude();
    void setExclude();
    void selectRouting();
    void actApply();
public slots:
    void clearFilter();

};

