#include "trackdiagramdata.h"

#include "data/train/train.h"
#include "data/train/routing.h"
#include "data/diagram/trainline.h"

TrackDiagramData::TrackDiagramData(const events_t& data,
    const QList<QString>& initTrackOrder) :
    data(data), initTrackOrder(initTrackOrder)
{
    //self._parseInitTrackOrder(init_tracks)
    _makeList();
    //? if not init_tracks:
    //?     self.track_order.sort()
}

std::shared_ptr<Track> TrackDiagramData::trackByName(const QString &name) const
{
    if (_doubleLine){
        if(auto p=downTracks.findTrack(name))
            return p;
        else return upTracks.findTrack(name);
    }else{
        return singleTracks.findTrack(name);
    }
}

void TrackDiagramData::refreshData()
{
    _makeList();
}

void TrackDiagramData::_makeList()
{
    if (_manual) {
        _allowMainStay = true;
        _doubleLine = false;
    }
    else {
        trackOrder.clear();
    }
    msg.clear();
    items.clear();
    singleTracks.clear();
    downTracks.clear();
    upTracks.clear();

    // 交路的后续列车：列车->LinkItem的映射
    std::map<const Train*, TrackItem*> postTrainMap;

    // 交路前序列车：对象->对应后续列车
    std::map<events_t::value_type, const Train*> preTrainMap;

    // 第一轮处理：数据格式转换
    for (const auto& pa : data) {
        convertItem(pa, postTrainMap, preTrainMap);
    }

    // 补充处理：LinkItem之间：把TrainStationPtr加到后续Item中
    for (auto& p : preTrainMap) {
        if (auto itr = postTrainMap.find(p.second); itr != postTrainMap.end()) {
            // 找到
            itr->second->trainStation2 = &*(p.first.second->trainStation);
        }
        else {
            qDebug() << "TrackDiagramData::_makeList: WARNING: " <<
                "Unexpected null data while looking for post-order-item: pre order is "
                << p.first.first->train()->trainName().full() << Qt::endl;
        }
    }

    // 第二轮处理：铺画  跨日的处理暂时不要表达在数据上
    foreach(auto it, items) {
        if (it->isStopped()) {
            _addStopTrain(it);
        }
        else {
            _addPassTrain(it);
        }
    }
    _autoTrackNames();
    _autoTrackOrder();
}

void TrackDiagramData::convertItem(events_t::const_reference pa, 
    std::map<const Train*, TrackItem*>& postTrainMap,
    std::map<events_t::value_type, const Train*>& preTrainMap)
{
    auto line = pa.first;
    auto train = line->train();
    const auto& ts = pa.second->trainStation;
    auto* pts=&*ts;
    //始发
    if (line->isStartingStation(pa.second)) {
        bool linkFlag = false;
        if (auto rt = train->routing().lock()) {
            // 存在交路
            if (auto* pre = rt->preLinked(*train)) {
                // 本车是接续的后车
                auto it = std::make_shared<TrackItem>(QString("%1-%2")
                    .arg(pre->name(), train->trainName().full()),
                    pa.second->trainStation->name,
                    pre->train()->lastStation()->arrive,
                    train->firstStation()->depart, TrackItem::Link,
                    line, pa.second->trainStation->track, pts);
                items.push_back(it);
                postTrainMap.emplace(train.get(), it.get());
                linkFlag = true;
            }
        }
        if (!linkFlag) {
            // 按照普通的始发进行处理
            auto it = std::make_shared<TrackItem>(train->trainName().full(),
                pa.second->trainStation->name, pa.second->trainStation->arrive,
                pa.second->trainStation->depart, TrackItem::Departure, line,
                pa.second->trainStation->track, pts);
            items.push_back(it);
        }
    }
    else if (line->isTerminalStation(pa.second)) {
        // 终到 如果有要link的，不处理；否则按正常终到处理
        bool linkFlag = false;
        if (auto rt = train->routing().lock()) {
            if (auto post = rt->postLinked(*train)) {
                preTrainMap.emplace(pa, post->train().get());
                linkFlag = true;
            }
        }
        if (!linkFlag) {
            auto it = std::make_shared<TrackItem>(train->trainName().full(),
                pa.second->trainStation->name, ts->arrive, ts->depart,
                TrackItem::Destination, line, ts->track, pts);
            items.push_back(std::move(it));
        }
    }
    else {
        auto tp = (ts->isStopped() ? TrackItem::Stop : TrackItem::Pass);
        items.push_back(std::make_shared<TrackItem>(train->trainName().full(),
            ts->name, ts->arrive, ts->depart, tp, line, ts->track, pts));
    }
}

void TrackDiagramData::_addPassTrain(std::shared_ptr<TrackItem> item)
{
    if (_manual && !item->specTrack.isEmpty()) {
        // 手动铺画
        auto track = singleTracks.trackByName(item->specTrack);
        if (auto itr = track->conflictItem(item, _sameSplitSecs, _oppositeSiteSplitSecs);
            itr != track->cend()) {
            // 发生冲突，提出警告，但不进行任何别的操作
            msg.push_back(QObject::tr("通过时刻冲突：尝试添加[%1] [%2] 至股道 [%3]，"
                "但与 [%4]冲突。").arg(item->title, item->beginTime.toString("hh:mm:ss"),
                    item->specTrack, itr->item->title));
        }
        track->addItem(item);
    }
    else if (_doubleLine) {
        // 经典模式：双线铺画  通过列车不会忽略正线
        if (item->dir == Direction::Down) {
            downTracks.autoAddDouble(item, false, _sameSplitSecs);
        }
        else {
            upTracks.autoAddDouble(item, false, _sameSplitSecs);
        }
    }
    else {
        // 单线模式
        singleTracks.autoAddSingle(item, false, _sameSplitSecs, _oppositeSiteSplitSecs);
    }
}

void TrackDiagramData::_addStopTrain(std::shared_ptr<TrackItem> item)
{
    if (_manual && !item->specTrack.isEmpty()) {
        // 手动
        auto track = singleTracks.trackByName(item->specTrack);
        if (auto itr = track->conflictItem(item, _sameSplitSecs, _oppositeSiteSplitSecs);
            itr != track->cend()) {
            // 发生冲突，提出警告，但不进行任何别的操作
            msg.push_back(QObject::tr("停车时刻冲突：尝试添加[%1] [%2]-[%3] 至股道 [%3]，"
                "但与 [%4]冲突。").arg(item->title, item->beginTime.toString("hh:mm:ss"),
                    item->endTime.toString("hh:mm:ss"),
                    item->specTrack, itr->item->title));
        }
        track->addItem(item);
    }
    else if (_doubleLine) {
        // 双线
        if (item->dir == Direction::Down) {
            downTracks.autoAddDouble(item, !_allowMainStay, _sameSplitSecs);
        }
        else {
            upTracks.autoAddDouble(item, !_allowMainStay, _sameSplitSecs);
        }
    }
    else {
        // 单线
        singleTracks.autoAddSingle(item, !_allowMainStay,
            _sameSplitSecs, _oppositeSiteSplitSecs);
    }
}

void TrackDiagramData::_autoTrackNames()
{
    if(_manual){
        singleTracks.autoNameManual();
    }else if(_doubleLine){
        downTracks.autoNameDown();
        upTracks.autoNameUp();
    }else{
        singleTracks.autoNameSingle();
    }
}

void TrackDiagramData::_autoTrackOrder()
{
    if (_manual) {
        // 只有手动模式下才解析原有的Order
        foreach(const auto& name, initTrackOrder){
            trackOrder.push_back(singleTracks.trackByName(name));
        }
//        if(_doubleLine){
//            // 这是原来不存在的分支  双线+手动铺画尝试
//            for (auto p = downTracks.tracks().crbegin();
//                p != downTracks.tracks().crend(); ++p) {
//                if(!trackOrder.contains((*p))){
//                     trackOrder.push_back((*p));
//                }
//            }
//            foreach(const auto & p, upTracks.tracks()) {
//                if(!trackOrder.contains(p->name())){
//                    trackOrder.push_back(p->name());
//                }
//            }
//        }

            foreach(auto p, singleTracks.tracks()) {
                if (!trackOrder.contains(p)) {
                    trackOrder.push_back(p);
                }
            }


    }
    else {
        // 单双线自动排序 ..
        trackOrder.clear();
        if (_doubleLine) {
            for (auto p = downTracks.tracks().crbegin();
                p != downTracks.tracks().crend(); ++p) {
                trackOrder.push_back(*p);
            }
            foreach(const auto & p, upTracks.tracks()) {
                trackOrder.push_back(p);
            }
        }
        else {
            foreach(const auto & p, singleTracks.tracks()) {
                trackOrder.push_back(p);
            }
        }
    }
}

std::shared_ptr<Track> TrackGroup::trackByName(const QString& name)
{
    if (auto t = findTrack(name)) {
        return t;
    }
    auto track = std::make_shared<Track>(name);
    _tracks.push_back(track);
    return track;
}

void TrackGroup::autoAddDouble(std::shared_ptr<TrackItem> item, bool ignoreMainTrack,
    int sameSplitSecs)
{
    auto p = _tracks.begin();
    if (ignoreMainTrack) {
        ensureOneTrack();
        p = std::next(_tracks.begin());
    }
    for (; p != _tracks.end(); ++p) {
        if ((*p)->isIdleForDouble(item, sameSplitSecs)) {
            break;
        }
    }
    if (p == _tracks.end()) {
        _tracks.push_back(std::make_shared<Track>());
        p = std::prev(_tracks.end());
    }
    (*p)->addItem(item);
}

void TrackGroup::autoAddSingle(std::shared_ptr<TrackItem> item, bool ignoreMainTrack,
    int sameSplitSecs, int oppsiteSplitSecs)
{
//    qDebug()<<"autoAddSingle: "<<DirFunc::dirToString(item->dir)<<", "
//        <<item->title<<Qt::endl;
    auto p = _tracks.begin();
    if (ignoreMainTrack) {
        ensureOneTrack();
        p = std::next(_tracks.begin());
    }
    for (; p != _tracks.end(); ++p) {
        if ((*p)->isIdleFor(item, sameSplitSecs, oppsiteSplitSecs)) {
            break;
        }
    }
    if (p == _tracks.end()) {
        _tracks.push_back(std::make_shared<Track>());
        p = std::prev(_tracks.end());
    }
    (*p)->addItem(item);
}

void TrackGroup::clear()
{
    _tracks.clear();
}

void TrackGroup::autoNameManual()
{
    int i=0;
    foreach(const auto& p,_tracks){
        if (p->name().isEmpty()){
            p->setName(validNewName(++i));
        }
    }
}

void TrackGroup::autoNameDown()
{
    int i=1;
    foreach(auto p,_tracks){
        p->setName(QString::number(i));
        i+=2;
    }
    if(!_tracks.empty()){
        _tracks.front()->setName("Ⅰ");
    }
}

void TrackGroup::autoNameUp()
{
    int i=2;
    foreach(auto p,_tracks){
        p->setName(QString::number(i));
        i+=2;
    }
    if(!_tracks.empty()){
        _tracks.front()->setName("Ⅱ");
    }
}

void TrackGroup::autoNameSingle()
{
    int i=1;
    foreach(auto p,_tracks){
        p->setName(QString::number(i++));
    }
    if(!_tracks.empty()){
        _tracks.front()->setName("Ⅰ");
    }
}

std::shared_ptr<Track> TrackGroup::findTrack(const QString& name) const
{
    foreach(auto t, _tracks) {
        if (t->name() == name)
            return t;
    }
    return nullptr;
}

QString TrackGroup::validNewName(int i)
{
    static const QString base="A%1";
    do{
        QString name=base.arg(i++);
        if (!findTrack(name))
            return name;
    }while(true);
}

void TrackGroup::ensureOneTrack()
{
    if(_tracks.empty())
        _tracks.push_back(std::make_shared<Track>());
}


