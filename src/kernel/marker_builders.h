#pragma once

#include <memory>

#include "data/train/linemarker.h"


class QGraphicsItem;
class QPen;
struct MarkerInfo;

/**
 * 2026.10.19: Abstract marker builder, base class of all marker builder classes.
 * We choose to user external information for storing the sizes.
 */
class AbstractMarkerBuilder 
{

public:
	virtual ~AbstractMarkerBuilder() = default;

	/**
	 * Main API for creating objects, of different actual types.
	 */
	static std::unique_ptr<AbstractMarkerBuilder> creeate(LineMarkerType type);

	/**
	 * Returns occupied length of one marker, along the running line direction.
	 * @param s: the configured size.
	 */
	virtual double occupied_length(double s)const = 0;

	/**
	 * Core function. Builds and returns the marker item.
	 * Return nullptr if the marker is not valid, or should not be built. The nullptr should be ignored by caller.
	 */
	virtual QGraphicsItem* build_marker(const MarkerInfo& info, double size, const QPen& pen, QGraphicsItem* parent=nullptr) = 0;

protected:
	AbstractMarkerBuilder() = default;
};

/**
 * Builder for a "null" marker. This should typically not be used. Nothing will be drawn.
 */
class NullMarkerBuilder : public AbstractMarkerBuilder
{
public:
	double occupied_length(double s)const override;
	QGraphicsItem* build_marker(const MarkerInfo& info, double size, const QPen& pen, QGraphicsItem* parent)override;
};

class SquareMarkerBuilder : public AbstractMarkerBuilder
{
public:
	double occupied_length(double s)const override;
	QGraphicsItem* build_marker(const MarkerInfo& info, double size, const QPen& pen, QGraphicsItem* parent)override;
};