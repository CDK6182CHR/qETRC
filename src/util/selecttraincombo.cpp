#include "selecttraincombo.h"
#include "data/train/train.h"
#include "data/train/traincollection.h"
#include <QtWidgets>

SelectTrainCombo::SelectTrainCombo(TrainCollection &coll_, QWidget *parent):
    QHBoxLayout(parent), coll(coll_)
{
    initUI();
}

SelectTrainCombo::SelectTrainCombo(TrainCollection &coll_, std::shared_ptr<Train> train,
                                   QWidget *parent):
    QHBoxLayout(parent), coll(coll_)
{
    initUI();
    setTrain(train);
}

std::shared_ptr<Train> SelectTrainCombo::dialogGetTrain(TrainCollection& coll_, 
    QWidget* parent, const QString& title,const QString& prompt)
{
    QDialog* dialog=new QDialog(parent);
    dialog->setWindowTitle(title);
    auto* vlay = new QVBoxLayout(dialog);
    if (!prompt.isEmpty()) {
        auto* lab = new QLabel(prompt);
        lab->setWordWrap(true);
        vlay->addWidget(lab);
    }

    auto* cb = new SelectTrainCombo(coll_);
    vlay->addLayout(cb);
    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vlay->addWidget(box);
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    auto t = dialog->exec();
    std::shared_ptr<Train> res{};
    if (t) {
        res = cb->train();
    }
    dialog->setParent(nullptr);
    delete dialog;
    return res;
}

void SelectTrainCombo::setTrain(std::shared_ptr<Train> train)
{
    if(train){
        matched.clear();
        matched.append(train);
        updateCombo();
    }
}

void SelectTrainCombo::resetTrain()
{
    matched.clear();
    cbTrains->clear();
    _train.reset();
    edName->clear();
}

void SelectTrainCombo::initUI()
{
    edName=new QLineEdit;
    edName->setToolTip(tr("请输入部分或完整车次，编辑结束后将在右侧下拉列表中显示"
            "符合条件的车次。"));
    connect(edName,&QLineEdit::editingFinished,
            this,&SelectTrainCombo::onEditingFinished);
    addWidget(edName,1);

    cbTrains=new QComboBox;
    connect(cbTrains,SIGNAL(currentIndexChanged(int)),
            this,SLOT(onIndexChanged(int)));
    addWidget(cbTrains,2);
}

void SelectTrainCombo::addTrainItem(std::shared_ptr<Train> t)
{
    cbTrains->addItem(tr("%1 (%2->%3)").arg(t->trainName().full())
                      .arg(t->starting().toSingleLiteral())
                      .arg(t->terminal().toSingleLiteral()));
}

void SelectTrainCombo::updateCombo()
{
    cbTrains->clear();
    foreach(auto t, matched){
        addTrainItem(t);
    }
    cbTrains->setFocus();
    if(cbTrains->count() > 1){
        cbTrains->showPopup();
    }
}

void SelectTrainCombo::onEditingFinished()
{
    const QString& text = edName->text();
    if (text.isEmpty())
        matched = coll.trains();    //copy contruct!
    else
        matched = coll.multiSearchTrain(text);
    updateCombo();
}

void SelectTrainCombo::onIndexChanged(int i)
{
    if (i >= 0 && i < matched.size()) {
        if (_train != matched.at(i)) {
            _train = matched.at(i);
            emit currentTrainChanged(_train);
        }
    }
    else {
        //qDebug()<<"SelectTrainCombo::onIndexChanged: WARNING: invalid index "
        //        <<i<<Qt::endl;
    }
}

void SelectTrainCombo::focusIn()
{
    edName->setFocus();
}

void SelectTrainCombo::setEnabled(bool on)
{
    edName->setEnabled(on);
    cbTrains->setEnabled(on);
}
