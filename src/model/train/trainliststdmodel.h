#pragma once

#include "model/general/qemoveablemodel.h"


class Train;

/**
 * 2025.01.31 Std model for a list of train, that supports moving and sorting.
 */
class TrainListStdModel : public QEMoveableModel
{
public:
	enum Columns {
		ColTrainName,
		ColStarting, 
		ColTerminal,
		ColType,
		ColShow,
		ColMile,
		ColMAX
	};

	TrainListStdModel(QObject* parent = nullptr);

	/**
	 * Insert a train into the model, serving at the last row.
	 */
	void addTrain(std::shared_ptr<Train> train);

	/**
	 * Returns an ordered vector of trains contained in this model.
	 */
	std::vector<std::shared_ptr<Train>> trains()const;
};