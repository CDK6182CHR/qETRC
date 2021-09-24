#include "railnet.h"
#include "data/rail/railway.h"
#include "data/rail/railcategory.h"


void RailNet::fromRailCategory(const RailCategory *cat)
{
    foreach(const auto& sub, cat->subCategories()){
        fromRailCategory(sub.get());
    }
    foreach(const auto& rail, cat->railways()){
        addRailway(rail.get());
    }
}

void RailNet::addRailway(const Railway *railway)
{
    std::shared_ptr<vertex> pre{};
    for(auto p=railway->firstDownInterval();p;p=railway->nextIntervalCirc(p)){
        if (!pre){
            pre=emplace_vertex(p->fromStation()->name, *(p->fromStation()));
        }
        auto post=emplace_vertex(p->toStation()->name, *(p->toStation()));
        emplace_edge(pre,post,railway->name(),*p);
        pre=std::move(post);
    }
}
