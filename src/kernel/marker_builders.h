#pragma once

#include <memory>

#include "data/train/linemarker.h"

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
};

class SquareMarkerBuilder : public AbstractMarkerBuilder
{
public:
	double occupied_length(double s)const override;
};