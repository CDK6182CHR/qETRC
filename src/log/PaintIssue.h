#pragma once

#include <memory>
#include <variant>

#include <QtLogging>
#include <QString>

#include "IssueInfo.h"

/**
 * 2023.08.14  The PaintIssue reports the warnings/errors during the painting process.
 * This is similar to previous DiagnosisIssue, but with different (independent) implementation.
 * The previous DiagnosisIssue is planned to be merged here in the future.
 * For safety, we use shared_ptr for now.
 */
struct PaintIssue {


	QtMsgType level;

	IssueInfo info;

	

	PaintIssue(QtMsgType level, const IssueInfo& info):
		level(level), info(info)
	{

	}


};

