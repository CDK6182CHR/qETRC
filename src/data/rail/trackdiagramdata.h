#pragma once

#include "railtrack.h"
#include <map>

struct DiagramOptions;

/**
 * @brief The TrackGroup class
 * 一组股道 （上行，或下行，或单线）的集合。包含查找、判定等算法。
 * 默认第一条判定为正线。
 */
class TrackGroup {
    QVector<std::shared_ptr<Track>> _tracks;
public:
    auto& tracks(){return _tracks;}

    /**
     * 用于手动铺画模式：根据所给股道名称搜索或创建股道
     */
    std::shared_ptr<Track> trackByName(const QString& name);

    std::shared_ptr<Track> idleTrack(const TrainTime& tm1,const TrainTime& tm2, bool ignoreMainTrack);

    /**
     * 已知双线，添加车次。只管添加Track对象，不管命名。
     */
    void autoAddDouble(std::shared_ptr<TrackItem> item, bool ignoreMainTrack,
                       int sameSplitMinu, int period_hours);

    void autoAddSingle(std::shared_ptr<TrackItem> item, bool ignoreMainTrack,
                       int sameSplitSecs, int oppsiteSplitSecs, int period_hours);

    void clear();

    void autoNameManual();
    void autoNameDown();
    void autoNameUp();
    void autoNameSingle();

    /**
     * @brief findTrack
     * 根据股道名称查找对象；找不到返回空
     * 目前实现为线性查找
     */
    std::shared_ptr<Track> findTrack(const QString& name)const;


private:
    /**
     * 返回一个合法的新股道名称，用于手动铺画模式。
     * i是起始数字下标，避免紧到重复计算。
     */
    QString validNewName(int i);
    void ensureOneTrack();
};


struct AdapterStation;
class TrainLine;
class Train;
class RailStation;
class Railway;
class Diagram;
/**
 * @brief The TrackDiagramData class  车站股道图 数据部分
 * pyETRC.StationGraphWidget 数据和算法部分
 * 原来的数据结构和算法太乱。重新设计。
 */
class TrackDiagramData
{
    using events_t=std::vector<std::pair<std::shared_ptr<TrainLine>,
        const AdapterStation*>>;
    const DiagramOptions& _ops;
    const events_t& data;
    QVector<std::shared_ptr<TrackItem>> items;
    QList<QString> initTrackOrder;
    QVector<std::shared_ptr<Track>> trackOrder;


    bool _doubleLine=false;   // 按双线铺画
    bool _allowMainStay=true;   // 正线停车准许
    bool _manual=true;      // 手动模式：使用给出的股道数据
    int _sameSplitSecs=0;  // 同向股道间隔 单位秒
    int _oppositeSiteSplitSecs=0;
    QStringList msg;    // 铺画警告信息

    TrackGroup downTracks, upTracks, singleTracks;

public:

    TrackDiagramData(const DiagramOptions& ops, const events_t& data, const QList<QString>& initTrackOrder);

    bool doubleLine()const{return _doubleLine;}
    int trackCount()const{return trackOrder.size();}
    const auto& getTrackOrder()const{return trackOrder;}
    auto& getTrackOrderRef(){return trackOrder;}

    /**
     * 由名称去查找Track；分为单双线的情况
     */
    std::shared_ptr<Track> trackByName(const QString& name)const;

    auto& diagramOptions()const { return _ops; }
    void refreshData();
    bool manual()const{return _manual;}
    void setDoubleLine(bool on){_doubleLine=on;}
    void setManual(bool on){_manual=on;}
    bool allowMainStay()const{return _allowMainStay;}
    void setAllowMainStay(bool on){_allowMainStay=on;}

    int sameSplitSecs()const{return _sameSplitSecs;}
    void setSameSplitSecs(int i){_sameSplitSecs=i;}
    int oppositeSplitSecs()const{return _oppositeSiteSplitSecs;}
    void setOppositeSplitSecs(int i){_oppositeSiteSplitSecs=i;}

    void setInitOrder(const QList<QString>& order){initTrackOrder=order;}

private:
    void _makeList();

    /**
     * 和pyETRC的_judgeType比较类似；判定类型，然后添加到列表。
     * 注意：所有的Link统一在后续的时候做！！
     */
    void convertItem(events_t::const_reference pa,
        std::map<const Train*, TrackItem*>& postTrainMap,
        std::map<events_t::value_type, const Train*>& preTrainMap
        );

    void _addPassTrain(std::shared_ptr<TrackItem> item, int period_hours);
    void _addStopTrain(std::shared_ptr<TrackItem> item, int period_hours);

    void _autoTrackNames();
    void _autoTrackOrder();
};
