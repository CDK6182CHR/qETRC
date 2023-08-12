#pragma once
#include <QWidget>
#include <QUndoCommand>
#include <memory>

#include "data/trainpath/trainpath.h"   // for unique_ptr


class QTableView;
class PathListModel;
class TrainPathCollection;
class QUndoStack;

class PathListWidget : public QWidget
{
	Q_OBJECT;
	TrainPathCollection& pathcoll;
	PathListModel* const _model;
	QTableView* table;
	QUndoStack* _undo;

public:

	/**
	 * The undoStack is used for commit operations directly.
	 * Note, the `undo` is nullable.
	 */
	PathListWidget(TrainPathCollection& pathcoll, QUndoStack* undo, QWidget* parent = nullptr);
	auto* model() { return _model; }

private:
	void initUI();

private slots:
	void actEdit();
	void actDelete();

public slots:
	void actAdd();

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

	class RemoveTrainPath :public QUndoCommand
	{
		int idx;
		PathListWidget*const pw;
		std::unique_ptr<TrainPath> path;
		bool first = true;
	public:
		RemoveTrainPath(int idx, PathListWidget* pw, QUndoCommand* parent = nullptr);
		virtual void undo()override;
		virtual void redo()override;
	};
}