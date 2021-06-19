#include <QtTest>
#include <QtCore>
#include <initializer_list>


#include "../../src/data/rail/rail.h"
#include "data/train/trainname.h"
#include "data/train/train.h"

class RailTest : public QObject
{
    Q_OBJECT

public:
    RailTest();
    ~RailTest();

private slots:
    void test_case1();
    void test_case2();
    void test_case3();

    //车次
    void test_case4();

    //车次读取
    void test_case5();

};

RailTest::RailTest()
{

}

RailTest::~RailTest()
{

}

void RailTest::test_case1()
{
    qDebug()<<"test begins...."<<Qt::endl;
    Railway railway(QObject::tr("宝成线"));
    qDebug()<<"name: "<<railway.name()<<Qt::endl;
    QStringList s;
    s<<"青白江"<<"L77"<<"新都"<<"成都";
    for(int i=0;i<4;i++){
        qDebug()<<"to append: "<<s.at(i)<<Qt::endl;
        railway.appendStation(StationName(s.at(i)),i*10,4,i*10,
                              i==1?PassedDirection::DownVia:PassedDirection::BothVia);
    }
    qDebug()<<"Add finished!"<<Qt::endl;
    railway.showStations();
    railway.showIntervals();

    qDebug()<<"REVERSE"<<Qt::endl;
    railway.reverse();
    railway.showStations();
    railway.showIntervals();
}

void RailTest::test_case2()
{
    return;
    auto t=QString(R"(D:\Python\train_graph\source\陇海线西局段.json)");
    QFile f(t);
    f.open(QFile::ReadOnly);
    auto contents=f.readAll();
    QJsonDocument doc=QJsonDocument::fromJson(contents);
    QJsonObject obj=doc.object().value("line").toObject();

    Railway railway(obj);
    railway.showStations();
    railway.showIntervals();
    railway.getRuler(0).show();

    qDebug()<<"REVERSED..."<<Qt::endl;
    railway.reverse();
    railway.showStations();
    railway.showIntervals();

    f.close();
}

void RailTest::test_case3()
{
    return;
    auto t=QString(R"(D:\Python\train_graph\source\京局410\京广线京石段.json)");
    QFile f(t);
    f.open(QFile::ReadOnly);
    auto contents=f.readAll();
    QJsonDocument doc=QJsonDocument::fromJson(contents);
    QJsonObject obj=doc.object().value("line").toObject();
    Railway railway(obj);

    railway.getForbid(0).show();
    f.close();
}

void RailTest::test_case4()
{
    QStringList s;
    s<<"Z218/5"<<" K1139/40"<<"3256/7A"<<"57418/5 (加油)";
    for(auto t:s){
        TrainName n(t);
        n.show();
    }
}

void RailTest::test_case5()
{
    auto t=QString(R"(D:\Python\train_graph\source\成贵客专线D20200701.pyetgr)");
    QFile f(t);
    f.open(QFile::ReadOnly);
    auto contents=f.readAll();
    QJsonDocument doc=QJsonDocument::fromJson(contents);
    QJsonObject obj=doc.object().value("trains").toArray().at(36).toObject();

    QJsonObject obj1=doc.object().value("line").toObject();
    auto railway=std::make_shared<Railway>(obj1);

    Train train(obj);

    train.bindToRailway(railway);
    train.show();
}

QTEST_APPLESS_MAIN(RailTest)

#include "tst_railtest.moc"
