#include <QtTest>
#include <QtCore>
#include <initializer_list>
#include <tuple>


#include "../../src/data/rail/rail.h"
#include "data/train/trainname.h"
#include "data/train/train.h"
#include "data/diagram/trainadapter.h"

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

    //车次绑定基本
    void test_case5();

    void test_case6();

    /*
     * 测试带有折返的车次绑定
     */
    void test_case7();

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
    return;
    auto t=QString(R"(D:\Python\train_graph\source\成贵客专线D20200701.pyetgr)");
    QFile f(t);
    f.open(QFile::ReadOnly);
    auto contents=f.readAll();
    QJsonDocument doc=QJsonDocument::fromJson(contents);
    QJsonObject obj=doc.object().value("trains").toArray().at(54).toObject();

    QJsonObject obj1=doc.object().value("line").toObject();
    auto railway=std::make_shared<Railway>(obj1);

    auto train=std::make_shared<Train>(obj);

    Config config;

    TrainAdapter adp(train,railway,config);
    adp.print();

}

void RailTest::test_case6()
{
    return;
    //车次拼接
    Train t1(TrainName("K454/1"));
    Train t2(TrainName("K454/1"));
    Train t3(TrainName("K451/4"));
    std::initializer_list<std::tuple<QString,QString,QString>> i1{
        {"绵阳","01:04","01:06"},
        {"皂角铺","01:20","01:20"},
        {"罗江","01:30","01:30"}
    };
    std::initializer_list<std::tuple<QString,QString,QString>> i2{
        {"皂角铺","01:20","01:20"},
        {"罗江","01:30","01:30"},
        {"德阳","01:40","01:50"}
    };
    std::initializer_list<std::tuple<QString,QString,QString>> i3{
        {"德阳","01:40","01:50"},
        {"广汉","02:00","02:04"}
    };
    for(const auto& t:i1){
        t1.appendStation(StationName::fromSingleLiteral(std::get<0>(t)),
                         QTime::fromString(std::get<1>(t),"hh:mm"),
                         QTime::fromString(std::get<2>(t),"hh:mm"));
    }
    for(const auto& t:i2){
        t2.appendStation(StationName::fromSingleLiteral(std::get<0>(t)),
                         QTime::fromString(std::get<1>(t),"hh:mm"),
                         QTime::fromString(std::get<2>(t),"hh:mm"));
    }
    for(const auto& t:i3){
        t3.appendStation(StationName::fromSingleLiteral(std::get<0>(t)),
                         QTime::fromString(std::get<1>(t),"hh:mm"),
                         QTime::fromString(std::get<2>(t),"hh:mm"));
    }
    Train t4(t1);
    Train t5(t1);
    Train t6(t2);
    Train t7(t1);
    qDebug()<<"Test 1: joint with cover"<<Qt::endl;
    t1.jointTrain(std::move(t2),false);
    t1.show();
    qDebug()<<"Test 2: joint with no cover"<<Qt::endl;
    t4.jointTrain(std::move(t3),false);
    t4.show();
    qDebug()<<"Test 3: reverse joint with cover"<<Qt::endl;
    t6.jointTrain(std::move(t5),true);
    t6.show();
    qDebug()<<"Test 4: joint with this covered by another"<<Qt::endl;
    t7.show();
    t1.show();
    qDebug()<<"AFTER"<<Qt::endl;
    t7.jointTrain(std::move(t1), false);
    t7.show();

}

void RailTest::test_case7()
{
    auto t=QString(R"(D:\Python\train_graph\source\京沪线上局段20191230.pyetgr)");
    QFile f(t);
    f.open(QFile::ReadOnly);
    auto contents=f.readAll();
    QJsonDocument doc=QJsonDocument::fromJson(contents);
    QJsonObject obj=doc.object().value("trains").toArray().at(799).toObject();

    QJsonObject obj1=doc.object().value("line").toObject();
    auto railway=std::make_shared<Railway>(obj1);

    auto train=std::make_shared<Train>(obj);

    Config config;
    config.max_passed_stations = 0;

    TrainAdapter adp(train,railway,config);
    adp.print();
}

QTEST_APPLESS_MAIN(RailTest)

#include "tst_railtest.moc"
