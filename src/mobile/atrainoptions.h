#pragma once
#ifdef QETRC_MOBILE

#include <QWidget>
#include <memory>

class Train;
class QLineEdit;
class Diagram;
class ATrainOptions : public QWidget
{
    Q_OBJECT
    Diagram& diagram;
    QLineEdit* edSearch;
public:
    explicit ATrainOptions(Diagram& diagram, QWidget *parent = nullptr);
private:
    void initUI();

signals:
    void actTrainEvent();
    void actTrainLines();
    void actRulerRef();
    void actDiagnosis();
    void focusInTrain(std::shared_ptr<Train>);
private slots:
    void actSearch();
};

#endif // ATRAINOPTIONS_H
