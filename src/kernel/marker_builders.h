#pragma once

#include <memory>
#include <QPointF>

#include "data/train/linemarker.h"

/**
 * 2026.10.19: Abstract marker builder, base class of all marker builder classes.
 * We choose to user external information for storing the sizes.
 */
class AbstractMarkerBuilder 
{
	QPointF m_center;

public:
	virtual ~AbstractMarkerBuilder() = default;

	/**
	 * Returns the center of the marker. The occupied running line section must have the same length
	 * before and after this point, along the running line.
	 */
	auto& center()const { return m_center; }

	/**
	 * Main API for creating objects, of different actual types.
	 */
	static std::unique_ptr<AbstractMarkerBuilder> creeate(LineMarkerType type, QPointF center);

	/**
	 * Returns occupied length of one marker, along the running line direction.
	 * @param s: the configured size.
	 */
	virtual double occupied_length(double s)const = 0;

protected:
	AbstractMarkerBuilder(QPointF center);
};

/**
 * Builder for a "null" marker. This should typically not be used. Nothing will be drawn.
 */
class NullMarkerBuilder : public AbstractMarkerBuilder
{
public:
	using AbstractMarkerBuilder::AbstractMarkerBuilder;
	double occupied_length(double s)const override;
};

class SquareMarkerBuilder : public AbstractMarkerBuilder
{
public:
	using AbstractMarkerBuilder::AbstractMarkerBuilder;
	double occupied_length(double s)const override;
};