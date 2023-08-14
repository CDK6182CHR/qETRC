#pragma once

#include <memory>
#include <variant>

#include <QtLogging>
#include <QString>

#include "IssueInfo.h"

class Train;
class Railway;
class RailInterval;
class RailStation;
/**
 * 2023.08.14  The PaintIssue reports the warnings/errors during the painting process.
 * This is similar to previous DiagnosisIssue, but with different (independent) implementation.
 * The previous DiagnosisIssue is planned to be merged here in the future.
 * For safety, we use shared_ptr for now.
 */
struct PaintIssue {
	enum IssueType {
		NoIssueType,
		// insert new issue type here
		// ...
		OtherIssueType,
	};

	QtMsgType level;

	IssueType type;
	std::shared_ptr<Train> train;
	std::shared_ptr<Railway> rail;
	using pos_t = std::variant<std::shared_ptr<RailStation>,
		std::shared_ptr<RailInterval>>;
	pos_t pos;
	QString msg;

	PaintIssue(
		QtMsgType level, IssueType type,
		std::shared_ptr<Train> train, std::shared_ptr<Railway> rail,
		pos_t pos, const QString& msg
	) :level(level), type(type), train(train), rail(rail), pos(pos), msg(msg)
	{

	}

	QString issueTypeToString(IssueType type);
};

