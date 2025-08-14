#pragma once

#include <QDialog>

class QCompleter;
class TrainCollection;
class QLineEdit;
class QTextEdit;
class TrainTag;
class Train;
class QCheckBox;
class TrainTagListDirectModel;

/**
 * Add a tag to multiple trains by specifying their names.
 */
class BatchAddTrainTagDialog : public QDialog
{
	Q_OBJECT;
	TrainCollection& m_coll;
	TrainTagListDirectModel* m_completionModel;
	QCompleter* m_tagCompleter;
	QLineEdit* m_edTag;
	QTextEdit* m_edTrains;
	QTextEdit* m_edReport;
	QCheckBox* m_ckCompleteOnly;

public:
	BatchAddTrainTagDialog(TrainCollection& coll, TrainTagListDirectModel* completionModel, QWidget* parent = nullptr);

private:
	void initUI();

signals:
	void tagAdded(const QString&, const std::vector<std::shared_ptr<Train>>&);

private slots:
	void actApply();
};