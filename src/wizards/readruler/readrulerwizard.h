#pragma once

#ifndef QETRC_MOBILE_2

#include <QWizard>

#include "readrulerpageinterval.h"
#include "readrulerpagetrain.h"
#include "readrulerpageconfig.h"
#include "readrulerpagepreview.h"

class Diagram;

/**
 * @brief The ReadRulerWizard class
 * 标尺综合 （Ctrl+Shift+B）
 */
class ReadRulerWizard : public QWizard
{
    Q_OBJECT;
    Diagram& diagram;
    ReadRulerPageInterval* pgInterval;
    ReadRulerPageTrain* pgTrain;
    ReadRulerPageConfig* pgConfig;
    ReadRulerPagePreview* pgPreview;
public:
    enum {
        PageStart = 0,
        PageInterval,
        PageTrain,
        PageConfig,
        PagePreview,
        PageMAX
    };
    ReadRulerWizard(Diagram& diagram_, QWidget* parent=nullptr);
    virtual void initializePage(int id)override;

private :
    void initUI();
    void initStartPage();
    void calculate();
signals:
    void rulerAdded(std::shared_ptr<Railway>, const QString& name);
    void rulerUpdated(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> data);

public slots:
    virtual void accept()override;
    virtual void reject()override;
};

#endif
