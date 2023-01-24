#pragma once
#include <optional>
#include <QString>

struct TrainIntervalStatResult
{
    struct RailResults{
        bool isValid;
        double totalMiles;
        double travelSpeed;
        double techSpeed;
        QString path_s;
    };

    int settledStationsCount;
    int settledStopCount;
    int totalSecs;
    int runSecs;
    int stopSecs;

    RailResults railResults;
};

