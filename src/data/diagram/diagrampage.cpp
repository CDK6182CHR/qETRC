#include "diagrampage.h"
#include "diagram.h"

DiagramPage::DiagramPage(Diagram &diagram,
                         const QList<std::shared_ptr<Railway> > &railways):
    _diagram(diagram),_railways(railways)
{

}

const Config& DiagramPage::config() const
{
    return _diagram.config();
}

const MarginConfig& DiagramPage::margins() const
{
    return _diagram.config().margins;
}

int DiagramPage::railwayIndex(const Railway& rail) const
{
    for (int i = 0; i < _railways.size(); i++) {
        if (&rail == _railways.at(i).get())
            return i;
    }
    return -1;
}

double DiagramPage::railwayStartY(const Railway& rail) const
{
    int idx = railwayIndex(rail);
    if (idx == -1)
        return 0;
    return _startYs[idx];
}
