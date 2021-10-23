#include "railrangecombo.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

#include <data/rail/railway.h>

RailRangeCombo::RailRangeCombo(QWidget *parent) : QWidget(parent)
{
    setContentsMargins(0,0,0,0);
    initUI();
}

void RailRangeCombo::refreshData()
{
    cbStart->clear();
    cbEnd->clear();
    if (!railway)
        return;
    //暂定UserData里面放绝对的index。
    //即使是放shared_ptr，如果外面改了数据，还是不安全...
    for(int i=0;i<railway->stations().size();i++){
        auto st=railway->stations().at(i);
        cbStart->addItem(st->name.toSingleLiteral(), i);
        // 第一个插入的操作，同时触发cbEnd的初始化操作
    }
}

void RailRangeCombo::initUI()
{
    auto* hlay=new QHBoxLayout(this);
    hlay->setContentsMargins(0,0,0,0);
    auto* cb=new QComboBox;
    cb->setEditable(false);
    cbStart=cb;
    hlay->addWidget(cb);
    auto* lab=new QLabel("-");
    lab->setFixedWidth(20);
    lab->setAlignment(Qt::AlignCenter);
    hlay->addWidget(lab);
    cb=new QComboBox;
    cb->setEditable(false);
    hlay->addWidget(cb);
    cbEnd=cb;
    connect(cbStart,qOverload<int>(&QComboBox::currentIndexChanged),
            this,&RailRangeCombo::onStartIndexChanged);
    connect(cbEnd,qOverload<int>(&QComboBox::currentIndexChanged),
            this,&RailRangeCombo::onEndIndexChanged);
}

void RailRangeCombo::setRailway(std::shared_ptr<Railway> railway)
{
    if (this->railway!=railway){
        this->railway=railway;
        refreshData();
    }
}

void RailRangeCombo::onStartIndexChanged(int i)
{
    if (i>=0 && i<railway->stations().size()){
        _start=railway->stations().at(i);
        emit startStationChanged(_start);

        int endIndex=cbEnd->currentIndex();
        int endCount=cbEnd->count();
        cbEnd->blockSignals(true);
        cbEnd->clear();
        // 无论什么情况，先把end那边重设一下
        for(int j=i;j<railway->stations().size();j++){
            auto sj=railway->stations().at(j);
            cbEnd->addItem(sj->name.toSingleLiteral(),j);
        }
        //合法性判断以及新的选择，利用到末尾的距离完成
        if (cbStart->count()-i >= endCount-endIndex){
            // 原来选择的是合法的  这同时保证_end非空
            cbEnd->setCurrentIndex(cbEnd->count()-(endCount-endIndex));
            cbEnd->blockSignals(false);
            emit stationRangeChanged(_start, _end);
        }else{
            // 原来选择非法  这时调用endIndexChanged完成后续
            cbEnd->blockSignals(false);
            cbEnd->setCurrentIndex(cbEnd->count()-1);
        }
    }
}

void RailRangeCombo::onEndIndexChanged(int i)
{
    if (i>=0 && i<railway->stations().size()){
        _end=railway->stations().at(cbEnd->currentData().toInt());
        emit endStationChanged(_end);
        // end改变了，start那边一定合法
        emit stationRangeChanged(_start,_end);
    }
}
