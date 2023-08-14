#include "IssueInfo.h"

#include "data/train/train.h"
#include "data/rail/railway.h"
#include "data/rail/railstation.h"
#include "data/rail/railinterval.h"

#include "data/diagram/trainevents.h"

QString IssueInfo::issueTypeToString(IssueType type)
{
	switch (type) {
	case WithdrawFirstBounding: return QObject::tr("撤销首站绑定");
	default: return "Invalid IssueType";
	}
}

QString IssueInfo::toString() const
{
	return QString("%1@%2 [%3] %4 %5").arg(
		train->trainName().full(),
		rail->name(),
		posString(),
		issueTypeToString(type),
		msg
	);
}

QString IssueInfo::posString() const
{
	if (std::holds_alternative<std::shared_ptr<const RailStation>>(pos)) {
		return std::get<std::shared_ptr<const RailStation>>(pos)->name.toSingleLiteral();
	}
	else {
		return std::get<std::shared_ptr<const RailInterval>>(pos)->toString();
	}
}

