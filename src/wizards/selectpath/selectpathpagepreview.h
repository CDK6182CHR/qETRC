#pragma once

#include <QWizardPage>

class QEControlledTable;
class QTextBrowser;
class QLineEdit;
class QTableView;
class PathOperationSeq;
class Railway;
class RailStationModel;
class RailNet;
/**
 * @brief The SelectPathPagePreview class
 * 预览页面。暂定同时显示经由。
 * seqDown, seqUp直接保存前面model中的引用。
 */
class SelectPathPagePreview : public QWizardPage
{
    Q_OBJECT
    const RailNet& net;
    std::shared_ptr<Railway> railway{};
    const PathOperationSeq& seqDown;
    const PathOperationSeq& seqUp;

    RailStationModel*const model;
    QEControlledTable* ctable;
    QTableView* table;
    QLineEdit* edName;
    QTextBrowser* edPath;
public:
    SelectPathPagePreview(const RailNet& net,
                          const PathOperationSeq& seqDown,
                          const PathOperationSeq& seqUp,
                          QWidget* parent=nullptr);


    /**
     * 初始化页面数据：计算径路和经由。
     */
    void setupData(bool withRuler, int rulerCount);

    void refreshData();

    auto getRailway(){return railway;}

    /**
     * @brief applyChange
     * 保存并报告结果。
     */
    bool applyChange();

private:
    void initUI();

private slots:
    void actSave();

};

