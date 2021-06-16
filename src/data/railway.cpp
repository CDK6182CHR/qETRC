#include "railway.h"



void Railway::fromJson(const QJsonObject &obj)
{
    _name=obj.value("name").toString();
    _notes.fromJson(obj.value("notes").toObject());
    const QJsonArray& ar=obj.value("stations").toArray();
    for(auto t=ar.cbegin();t!=ar.cend();t++){
        _stations.append(std::make_shared<RailStation>(t->toObject()));
    }

    /*
     * TODOs:

        try:
            self.rulers
        except:
            self.rulers = []
        for ruler_dict in origin["rulers"]:
            new_ruler = Ruler(origin=ruler_dict,line=self)
            self.rulers.append(new_ruler)
        for route_dict in origin.get('routes',[]):
            r = Route(self)
            r.parseData(route_dict)
            self.routes.append(r)

        self.forbid.loadForbid(origin.get("forbid",None))
        self.forbid2.loadForbid(origin.get("forbid2",None))
        self.setNameMap()
        self.setFieldMap()
        self.verifyNotes()
        self.resetRulers()
      */
}
