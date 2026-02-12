#pragma once
#include <optional>
#include <QString>

struct TrainIntervalStatResult
{
    struct RailResults{
        bool isValid = false;
        double totalMiles = 0;
        double travelSpeed = 0;
        double techSpeed = 0;
        QString path_s;
    };

    int settledStationsCount = 0;
    int settledStopCount = 0;
    int totalSecs = 0;
    int runSecs = 0;
    int stopSecs = 0;

    RailResults railResults;
};

