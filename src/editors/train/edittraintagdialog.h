#pragma once

#include <memory>
#include <QDialog>

class TrainTag;
class QLineEdit;
class TrainTagManager;
class QTextEdit;


/**
 * Dialog for adding/editing train dialog.
 * The input TrainTag object may be nullptr or not, for adding new tag and editing existing tag, respectively.
 */
class EditTrainTagDialog : public QDialog
{
	Q_OBJECT;
	std::shared_ptr<TrainTag> m_tag;
	TrainTagManager& m_manager;  // For name confilicting check

	QLineEdit* m_edName;
	QTextEdit* m_edNote;

public:

	// For existing tags
	EditTrainTagDialog(std::shared_ptr<TrainTag> tag, TrainTagManager& manager, QWidget* parent = nullptr);

	EditTrainTagDialog(TrainTagManager& manager, QWidget* parent = nullptr);

private:
	void initUI();

signals:

	// Train name and/or note changed
	void tagNameChanged(std::shared_ptr<TrainTag> tag, std::shared_ptr<TrainTag> data);

	// Only note changed (for this case, we need to do fewer things than nameChanged)
	void tagNoteChanged(std::shared_ptr<TrainTag> tag, std::shared_ptr<TrainTag> data);

	// New tag added
	void tagAdded(std::shared_ptr<TrainTag> tag);

public slots:
	void refreshData();

	void accept()override;

};
