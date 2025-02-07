#pragma once

#include <memory>
#include <QString>

class Ruler;

/**
 * A segment of the ruler for a TrainPath, corresponding to the TrainPathSeg.
 * The segment is valid iff the `ruler` holds a valid object.
 */
struct PathRulerSeg {
	QString ruler_name;
	std::weak_ptr<Ruler> ruler;
};