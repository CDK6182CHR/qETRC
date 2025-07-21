#pragma once

#include <QWizardPage>
#include <QStandardItemModel>
#include <memory>

class QTableView;
class Train;
class Ruler;
class Railway;
struct DiagramOptions;

class TimeInterpPreviewModel: public QStandardItemModel
{
    Q_OBJECT;
    const DiagramOptions& _ops;
public:
    enum {
        ColName=0,
        ColType,
        ColStarting,
        ColTerminal,
        ColError,
        ColMAX
    };
    TimeInterpPreviewModel(const DiagramOptions& ops, QObject* parent=nullptr);
    void setupModel(std::shared_ptr<Railway> railway,
                    std::shared_ptr<Ruler> ruler,
                    const QVector<std::shared_ptr<Train>> trains);
};

class TimeInterpPagePreview : public QWizardPage
{
    Q_OBJECT;
    const DiagramOptions& _ops;
    TimeInterpPreviewModel* const model;
    QTableView* table;
public:
    TimeInterpPagePreview(const DiagramOptions& ops, QWidget* parent=nullptr);
    void setupData(std::shared_ptr<Railway> railway,
                   std::shared_ptr<Ruler> ruler,
                   const QVector<std::shared_ptr<Train>> trains);
    auto* getModel(){return model;}
private:
    void initUI();
};

