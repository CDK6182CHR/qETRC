#include "raildbitems.h"
#include "data/rail/railcategory.h"
#include "data/rail/railway.h"

navi::RailCategoryItem::RailCategoryItem(std::shared_ptr<RailCategory> cat,
                                         int row, RailCategoryItem *parent):
    navi::AbstractComponentItem(row,parent), _category(cat)
{
    int r=0;
    foreach(auto p, cat->subCategories()){
        _subcats.emplace_back(std::make_unique<RailCategoryItem>(p,r++,this));
    }
    foreach(auto p,cat->railways()){
        _railways.emplace_back(std::make_unique<RailwayItemDB>(p,r++,this));
    }
}

navi::AbstractComponentItem *navi::RailCategoryItem::child(int i)
{
    if (i<static_cast<int>(_subcats.size())){
        return _subcats.at(i).get();
    }else{
        return _railways.at(i-_subcats.size()).get();
    }
}

int navi::RailCategoryItem::childCount() const
{
    return _subcats.size()+_railways.size();
}

QString navi::RailCategoryItem::data(int i) const
{
    switch (i)
    {
    case navi::RailCategoryItem::DBColName: return _category->name();
        break;
    case navi::RailCategoryItem::DBColMile:return QString::number(childCount());
        break;
    default:return {};
        break;
    }
}

int navi::RailCategoryItem::subCategoriesCount() const
{
    return _subcats.size();
}

void navi::RailCategoryItem::removeRailwayAt(int i)
{
    int ri=i-_subcats.size();
    _category->railways().removeAt(ri);
    auto p=_railways.begin();std::advance(p,ri);
    p=_railways.erase(p);
    for(;p!=_railways.end();++p){
        (*p)->setRow((*p)->row()-1);
    }
}

void navi::RailCategoryItem::removeRailwaysAt(int i, int count)
{
    int ri = i - _subcats.size();
    auto p = _railways.begin(); std::advance(p, ri);
    for (int t = 0; t < count; t++) {
        p = _railways.erase(p);
        _category->railways().removeAt(ri);
    }
    for (; p != _railways.end(); ++p) {
        (*p)->setRow((*p)->row() - count);
    }
}

void navi::RailCategoryItem::insertRailwayAt(std::shared_ptr<Railway> rail, int i)
{
    int ri = i - _subcats.size();
    _category->railways().insert(ri,rail);
    auto p=_railways.begin();std::advance(p,ri);
    p=_railways.insert(p,std::make_unique<RailwayItemDB>(rail,i,this));
    for(++p;p!=_railways.end();++p){
        (*p)->setRow((*p)->row()+1);
    }
}

void navi::RailCategoryItem::insertRailwaysAt(const QList<std::shared_ptr<Railway>>& rails, int i)
{
    int ri = i - _subcats.size();
    auto p = _railways.begin();
    std::advance(p, ri);
    foreach(auto rail, rails) {
        _category->railways().insert(ri, rail);
        p = _railways.insert(p, std::make_unique<RailwayItemDB>(rail, i, this));
        ++p; ++i; ++ri;
    }

    for (; p != _railways.end(); ++p) {
        (*p)->setRow((*p)->row() + rails.size());
    }
}

void navi::RailCategoryItem::insertCategoryAt(std::shared_ptr<RailCategory> cat, int i)
{
    _category->subCategories().insert(i, cat);
    auto p = _subcats.begin(); std::advance(p, i);
    p = _subcats.insert(p, std::make_unique<RailCategoryItem>(cat, i, this));
    for (++p; p != _subcats.end(); ++p) {
        (*p)->setRow((*p)->row() + 1);
    }
    for (auto& pr : _railways) {
        pr->setRow(pr->row() + 1);
    }
}

void navi::RailCategoryItem::removeCategoryAt(int i)
{
    _category->subCategories().removeAt(i);
    auto p = _subcats.begin(); std::advance(p, i);
    p = _subcats.erase(p);
    for (; p != _subcats.end(); ++p) {
        (*p)->setRow((*p)->row() - 1);
    }
    for (auto& pr : _railways) {
        pr->setRow(pr->row() - 1);
    }
}

std::deque<navi::path_t> navi::RailCategoryItem::searchBy(
        std::function<bool (const Railway &)> pred)
{
    std::deque<path_t> res;
    for(const auto& subcat:_subcats){
        auto subres=subcat->searchBy(pred);
        res.insert(res.end(),subres.begin(),subres.end());
    }
    for(const auto& rail:_railways){
        if (pred(*(rail->railway()))){
            res.emplace_back(rail->path());
        }
    }
    return res;
}


navi::RailwayItemDB::RailwayItemDB(std::shared_ptr<Railway> rail,
                                   int row, RailCategoryItem *parent):
    navi::AbstractComponentItem(row,parent),_railway(rail)
{

}

QString navi::RailwayItemDB::data(int i) const
{
    switch (i)
    {
    case navi::AbstractComponentItem::DBColName: return _railway->name();
        break;
    case navi::AbstractComponentItem::DBColMile:
        return QString::number(_railway->railLength(), 'f', 3);
        break;
    case navi::AbstractComponentItem::DBColStart:
        return _railway->firstStationName().toSingleLiteral();
        break;
    case navi::AbstractComponentItem::DBColEnd:
        return _railway->lastStationName().toSingleLiteral();
        break;
    case navi::AbstractComponentItem::DBColAuthor:
        return _railway->notes().author;
        break;
    case navi::AbstractComponentItem::DBColVersion:
        return _railway->notes().version;
        break;
    default:return {};
    }
}
