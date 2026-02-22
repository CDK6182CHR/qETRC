#pragma once

#include <memory>
#include <variant>

#include <QString>
#include <QDebug>

class Train;
class Railway;
class RailInterval;
class RailStation;

/**
 * 2023.08.14  see also PaintIssue
 * The core information contained in PaintIssue class (without level)
 */
struct IssueInfo {
	enum IssueType {
		NoIssueType,

		


		WithdrawFirstBounding,
		InvalidPath,

		// 2026.02.22: issues for binding using paths
		PathBindSingleStation,
		PathBindSkipped,
		PathBindLateStart,
		PathBindEarlyStop,

		// insert new issue type here
		// ...
		OtherIssueType,
	};

	IssueType type;
	std::shared_ptr<Train> train;
	std::shared_ptr<Railway> rail;
	using pos_t = std::variant<std::shared_ptr<const RailStation>,
		std::shared_ptr<const RailInterval>>;
	pos_t pos;
	QString msg;

	static QString issueTypeToString(IssueType type);

	IssueInfo(
		IssueType type,
		std::shared_ptr<Train> train,
		std::shared_ptr<Railway> rail,
		pos_t pos,
		const QString& msg
	) : type(type), train(train), rail(rail), pos(pos), msg(msg)
	{

	}

	QString toString()const;

	QString posString()const;
};



