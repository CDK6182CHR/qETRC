#pragma once

#include <QWizardPage>
#include <QStandardItemModel>
#include <memory>

class QTableView;
class Train;
class Ruler;
class Railway;
class TimeInterpPreviewModel: public QStandardItemModel
{
    Q_OBJECT
public:
    enum {
        ColName=0,
        ColType,
        ColStarting,
        ColTerminal,
        ColError,
        ColMAX
    };
    TimeInterpPreviewModel(QObject* parent=nullptr);
    void setupModel(std::shared_ptr<Railway> railway,
                    std::shared_ptr<Ruler> ruler,
                    const QVector<std::shared_ptr<Train>> trains);
};

class TimeInterpPagePreview : public QWizardPage
{
    Q_OBJECT
    TimeInterpPreviewModel* const model;
    QTableView* table;
public:
    TimeInterpPagePreview(QWidget* parent=nullptr);
    void setupData(std::shared_ptr<Railway> railway,
                   std::shared_ptr<Ruler> ruler,
                   const QVector<std::shared_ptr<Train>> trains);
    auto* getModel(){return model;}
private:
    void initUI();
};

