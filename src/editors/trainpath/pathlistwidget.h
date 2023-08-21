#pragma once
#include <QWidget>
#include <QUndoCommand>
#include <memory>

#include "data/trainpath/trainpath.h"   // for unique_ptr


class QTableView;
class PathListModel;
class TrainPathCollection;
class QUndoStack;
class RailCategory;

class PathListWidget : public QWidget
{
	Q_OBJECT;
	RailCategory& railcat;
	TrainPathCollection& pathcoll;
	PathListModel* const _model;
	QTableView* table;
	QUndoStack* _undo;

public:

	/**
	 * The undoStack is used for commit operations directly.
	 * Note, the `undo` is nullable.
	 */
	PathListWidget(RailCategory& railcat, TrainPathCollection& pathcoll, QUndoStack* undo, QWidget* parent = nullptr);
	auto* model() { return _model; }

	/**
 	 * Called by RailContext upon the path is updated (by editor).
	 * Simple linear alg.
	 */
	void updatePath(TrainPath* path);

private:
	void initUI();

signals:
	void editPath(int index);

	/**
	 * Inform the RailCategory, to close corresponding docks
	 */
	void pathRemoved(TrainPath*);

	void focusInPath(TrainPath*);

	void removePath(int idx);

private slots:
	void actEdit();
	void actDelete();
	void editPathByModelIndex(const QModelIndex& idx);
	void onCurrentChanged(const QModelIndex& idx);

	/**
	 * 2023.08.21  Update (re-check) the validity of all paths.
	 * This does not cause any data change (by the current def.), so no undo stack operations.
	 * Just commit directly.
	 */
	void actUpdateAllValidity();

public slots:
	void actAdd();

	// call from navi
	//void actRemovePath(int idx);

	void actDuplicate(int idx);

	void refreshData();
public:

	void commitAddPath(std::unique_ptr<TrainPath>&& path);
	std::unique_ptr<TrainPath> undoAddPath();

	std::unique_ptr<TrainPath> commitRemovePath(int idx);
	void undoRemovePath(int idx, std::unique_ptr<TrainPath>&& path);
};

namespace qecmd {
	class AddTrainPath : public QUndoCommand
	{
		std::unique_ptr<TrainPath> path;
		PathListWidget* const pw;
	public:
		AddTrainPath(std::unique_ptr<TrainPath>&& path_, PathListWidget* const pw,
			QUndoCommand* parent = nullptr);
		virtual void undo()override;
		virtual void redo()override;
	};

}