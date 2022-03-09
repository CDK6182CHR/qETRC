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
    _logs.clear();


    //todo here ...
    return false;
}

void GreedyPainter::addLog(std::unique_ptr<CalculationLogAbstract> log)
{
    qDebug() << log->toString() << Qt::endl;
    _logs.emplace_back(std::move(log));
}
