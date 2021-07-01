#include "diagram.h"
#include "data/rail/railway.h"
#include "trainadapter.h"

#include <QFile>
#include <QJsonObject>


SystemJson SystemJson::instance;   //默认构造

void SystemJson::saveFile()
{
    static const QString filename = "system.json";
    QFile file(filename);
    file.open(QFile::WriteOnly);
    if (!file.isOpen()) {
        qDebug() << "SystemJson::saveFile: WARNING: open file " << filename << " failed." << Qt::endl;
        return;
    }
    QJsonDocument doc(toJson());
    file.write(doc.toJson());
    file.close();
}

void SystemJson::addHistoryFile(const QString& name)
{
    if (history.size() >= history_count)
        history.pop_front();
    history.push_back(name);
}

SystemJson::~SystemJson()
{
    saveFile();
}

SystemJson::SystemJson()
{
    static const QString filename = "system.json";
    QFile file(filename);
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) {
        qDebug() << "SystemJson::SystemJson: WARNING: system configuration file " << filename <<
            " not read. Use default." << Qt::endl;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    fromJson(doc.object());
    file.close();
}

void SystemJson::fromJson(const QJsonObject& obj)
{
    last_file = obj.value("last_file").toString();
    default_file = obj.value("default_file").toString(default_file);

    const QJsonArray& arhis = obj.value("history").toArray();
    for (const auto& p : arhis) {
        history.append(p.toString());
    }
}

QJsonObject SystemJson::toJson() const
{
    QJsonArray ar;
    for (const auto& p : history)
        ar.append(p);
    return QJsonObject{
        {"last_file",last_file},
        {"default_file",default_file},
        {"history",ar}
    };
}



void Diagram::addRailway(std::shared_ptr<Railway> rail)
{
    _railways.append(rail);
    for (auto p : trains()) {
        p->bindToRailway(*rail, _config);
    }
}

void Diagram::addTrains(const TrainCollection& coll)
{
    //复制语义
    for (auto p : coll.trains()) {
        if (!_trainCollection.trainNameExisted(p->trainName())) {
            auto t = std::make_shared<Train>(*p);
            _trainCollection.trains().append(t);
            updateTrain(t);
        }
    }
}

std::shared_ptr<Railway> Diagram::railwayByName(const QString &name)
{
    for(const auto& p:_railways){
        if(p->name() == name)
            return p;
    }
    return nullptr;
}

void Diagram::updateRailway(std::shared_ptr<Railway> r)
{
    for (const auto& p:_trainCollection.trains()){
        p->updateBoundRailway(*r, _config);
    }
}

void Diagram::updateTrain(std::shared_ptr<Train> t)
{
    for(const auto& r:_railways){
        t->updateBoundRailway(*r, _config);
    }
}

TrainEventList Diagram::listTrainEvents(const Train& train) const
{
    TrainEventList res;
    for (auto p : train.adapters()) {
        res.push_back(qMakePair(p, p->listAdapterEvents(_trainCollection)));
    }
    return res;
}

std::shared_ptr<DiagramPage> Diagram::createDefaultPage()
{
    auto t = std::make_shared< DiagramPage>(*this, _railways);
    _pages.append(t);
    return t;
}

void Diagram::bindAllTrains()
{
    for (auto p : _railways) {
        for (auto t : _trainCollection.trains()) {
            t->bindToRailway(*p, _config);
        }
    }
}

Diagram::Diagram()
{
    readDefaultConfigs("config.json");
}

bool Diagram::readDefaultConfigs(const QString& filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) {
        qWarning("Diagram::readDefaultConfigs: cannot open config file, will use built-in default.");
        _defaultManager.initDefaultTypes();
        return false;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject& obj = doc.object();
    _defaultConfig.fromJson(obj);
    _defaultManager.readForDefault(obj);
    return true;
}

bool Diagram::fromJson(const QString& filename)
{
    QFile f(filename);
    f.open(QFile::ReadOnly);
    if (!f.isOpen()) {
        qDebug() << "Diagram::fromJson: ERROR: open file " << filename << " failed. " << Qt::endl;
        return false;
    }
    auto contents = f.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(contents);
    bool flag = fromJson(doc.object());
    f.close();
    return flag;
}

bool Diagram::fromJson(const QJsonObject& obj)
{
    if (obj.empty())
        return false;
    _railways.clear();

    //车次和Config直接转发即可
    _trainCollection.fromJson(obj, _defaultManager);
    bool flag = _config.fromJson(obj.value("config").toObject());
    if (!flag) {
        //缺配置信息，使用默认值
        _config = _defaultConfig;
    }
    _note = obj.value("markdown").toString();
    _version = obj.value("version").toString();

    //线路  line作为第一个，lines作为其他，不存在就是空
    auto rail0 = std::make_shared<Railway>(obj.value("line").toObject());
    _railways.append(rail0);

    //其他线路
    const QJsonArray& arrail = obj.value("lines").toArray();
    for (const auto& r : arrail) {
        _railways.append(std::make_shared<Railway>(r.toObject()));
    }

    //特殊：旧版排图标尺 （优先级低于rail中的）
    const auto& t = obj.value("config").toObject().value("ordinate");
    if (t.isString() && !t.toString().isEmpty()) {
        _railways.at(0)->setOrdinate(t.toString());
    }
    bindAllTrains();
    return true;
}

QJsonObject Diagram::toJson() const
{
    //车次信息表
    QJsonObject obj = _trainCollection.toJson();
    //线路信息
    if (!_railways.isEmpty()) {
        auto p = _railways.begin();
        obj.insert("line", (*p)->toJson());
        ++p;
        if (p != _railways.end()) {
            QJsonArray arrail;
            for (; p != _railways.end(); ++p) {
                arrail.append((*p)->toJson());
            }
            obj.insert("lines", arrail);
        }
    }
    //配置信息
    QJsonObject objconfig = _config.toJson();
    _trainCollection.typeManager().toJson(objconfig);
    obj.insert("config", objconfig);
    obj.insert("markdown", _note);
    obj.insert("version", _version);
    return obj;
}

bool Diagram::saveAs(const QString& filename)
{
    _filename = filename;
    return save();
}

bool Diagram::save() const
{
    QFile file(_filename);
    file.open(QFile::WriteOnly);
    if (!file.isOpen()) {
        qDebug() << "Diagram::save: WARNING: open file " << _filename << " failed. Nothing todo."
            << Qt::endl;
        return false;
    }
    QJsonDocument doc(toJson());
    file.write(doc.toJson());
    file.close();
    return true;
}


