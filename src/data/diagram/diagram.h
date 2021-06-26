#pragma once

#include "data/common/common_header.h"
#include <memory>
#include <QList>
#include <QJsonObject>
#include <QString>
#include "config.h"
#include "data/train/traincollection.h"


class Train;
class Railway;

/**
 * @brief The Diagram class  运行图基础数据的最高层次抽象
 * pyETRC.Graph的扩展
 * 支持多Railway。
 * 暂定仅支持一套TrainCollection
 * 注意全程Train应当是和Railway绑定的状态
 */
class Diagram
{
    QList<std::shared_ptr<Railway>> _railways;
    TrainCollection _trainCollection;
    Config _config, _defaultConfig;
    QString _filename;
    QString _version, _note;
public:
    Diagram()=default;
    Diagram(const QJsonObject& obj);
    Diagram(const QString& filename);

    //拷贝和移动暂时禁用，需要时再考虑
    Diagram(const Diagram&)=delete;
    Diagram(Diagram&&)=delete;
    Diagram& operator=(const Diagram&)=delete;
    Diagram& operator=(Diagram&&)=delete;

    /**
     * @brief fromJson  清空既有数据，从文件读取，同时保存文件名
     */
    void fromJson(const QString& filename);

    /**
     * @brief fromJson  读入数据后绑定车次和线路
     */
    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    inline auto& railways(){return _railways;}
    inline const auto& railways()const{return _railways;}
    inline auto firstRailway() { return _railways.first(); }
    inline auto& trainCollection(){return _trainCollection;}
    inline const auto& trainCollection()const{return _trainCollection;}
    inline const QString& filename()const{return _filename;}
    inline void setFilename(const QString& n){_filename=n;}
    inline auto& config(){return _config;}
    inline const auto& config()const{return _config;}
    inline const QString& version()const { return _version; }
    inline void setVersion(const QString& v) { _version = v; }
    inline const QString& note()const { return _note; }
    inline void setNote(const QString& n) { _note = n; }

    /**
     * @brief addRailway 添加线路
     * 同时将所有车次绑定到新线路
     */
    void addRailway(std::shared_ptr<Railway> rail);

    std::shared_ptr<Railway> railwayByName(const QString& name);
    inline std::shared_ptr<const Railway>
        railwayByName(const QString& name)const{
        return const_cast<Diagram*>(this)->railwayByName(name);
    }
    int railwayCount()const { return _railways.size(); }

    /**
     * @brief updateRailway  铺画准备
     * 更新指定线路，与所有车次重新绑定
     */
    void updateRailway(std::shared_ptr<Railway> r);

    /**
     * @brief updateTrain  铺画准备
     * @param t  更新的车次，与所有线路重新绑定
     */
    void updateTrain(std::shared_ptr<Train> t);

    auto& trains() { return _trainCollection.trains(); }

    ~Diagram();

private:
    void bindAllTrains();
};


