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
    if (i<_subcats.size()){
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
