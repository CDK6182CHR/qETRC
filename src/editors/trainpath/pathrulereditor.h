#pragma once

#include <QWidget>

class PathRulerModel;
class TrainPath;
class PathRuler;

class QTableView;
class QLineEdit;
class QLabel;

/**
 * 2025.02.10  Experimental 
 * Editor widget for PathRuler. For both creating and modifying.
 * This editor is designed in a different way from previous ones. The PathRuler has TWO distinct objects:
 * (1) the data under edition, the `m_ruler`, managed by shared_ptr, which is the ruler that stored by the TrainPath;
 * (2) the object storing the temporary status of the edited one, the m_model->ruler(), managed by VALUE type.
 * The latter one is used for updating the data.
 */
class PathRulerEditor: public QWidget {
	Q_OBJECT
	PathRulerModel* m_model;

	/**
	 * This is the ruler object that is managed by this editor.
	 * Must not be null.
	 * This object is NOT shared with the model; the model stores a COPY of current this object.
	 */
	std::shared_ptr<PathRuler> m_ruler;

	QTableView* m_table;
	QLineEdit* m_edRulerName;
	QLineEdit* m_edPathName;
	QLabel* m_labValid;

public:
	PathRulerEditor(std::shared_ptr<PathRuler> ruler, QWidget* parent = nullptr);

	void refreshBasicData();
	
	void setRuler(std::shared_ptr<PathRuler> ruler);

private:
	void initUI();

public slots:
	void refreshData();
};