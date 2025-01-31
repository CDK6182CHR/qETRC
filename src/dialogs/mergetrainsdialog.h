#pragma once

#include <memory>
#include <QDialog>

class SelectTrainWidget;
class QCheckBox;
class TrainCollection;
class QLineEdit;
class Train;

/**
 * 2025.01.31  Merge multiple trains as one new train. 
 */
class MergeTrainsDialog : public QDialog
{
	Q_OBJECT
	TrainCollection& m_coll;
	SelectTrainWidget* m_selWidget;
	QCheckBox* m_ckUseNewName;
	QCheckBox* m_ckMergeCross;
	QLineEdit* m_edNewName;

public:
	MergeTrainsDialog(TrainCollection& coll, QWidget* parent = nullptr);

	void accept()override;
	void reject()override;

private:
	void initUI();

signals:
	void mergeApplied(std::vector<std::shared_ptr<Train>> trains, std::shared_ptr<Train> new_train);
};