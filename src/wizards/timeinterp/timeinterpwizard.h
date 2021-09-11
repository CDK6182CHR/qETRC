#pragma once
#include <QWizard>

class TimeInterpPagePreview;
class TimeInterpPageTrain;
class Diagram;
class Train;

/**
 * @brief The TimeInterpWizard class
 * 标尺推定通过站时刻的wizard
 */
class TimeInterpWizard : public QWizard
{
    Q_OBJECT
    Diagram& diagram;
    TimeInterpPageTrain* pgTrain;
    TimeInterpPagePreview* pgPreview;
    QVector<std::shared_ptr<Train>> trains;
public:
    TimeInterpWizard(Diagram& diagram, QWidget* parent=nullptr);
private:
    void initUI();

signals:
    void interpolationApplied(const QVector<std::shared_ptr<Train>>& trains,
        const QVector<std::shared_ptr<Train>>& data);

public slots:
    virtual void accept() override;

protected:
    virtual void initializePage(int id) override;
};

