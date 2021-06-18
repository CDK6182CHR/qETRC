#include <QtTest>

#include "../../src/data/rail/rail.h"

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

};

RailTest::RailTest()
{

}

RailTest::~RailTest()
{

}

void RailTest::test_case1()
{
    return;
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

    railway.addEmptyRuler(QObject::tr("特快"),true);
    railway.addEmptyRuler(QObject::tr("快速"),true);
    railway.getRuler(0).show();
    railway.getRuler(1).show();
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

    f.close();
}

void RailTest::test_case3()
{
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

QTEST_APPLESS_MAIN(RailTest)

#include "tst_railtest.moc"
