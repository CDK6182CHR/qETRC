#include "greedypainter.h"
#include <data/diagram/diagram.h>
#include <data/train/train.h>

GreedyPainter::GreedyPainter(Diagram &diagram):
    diagram(diagram)
{

}

bool GreedyPainter::paint(const TrainName &trainName)
{
    _railAxis=diagram.stationEventAxisForRail(_railway);
    _train=std::make_shared<Train>(trainName);
    //todo here ...
    return false;
}
