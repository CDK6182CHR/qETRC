#include "intervalconflictreport.h"
#include <QObject>


QString IntervalConflictReport::typeToString(ConflictType type)
{
	switch (type)
	{
	case IntervalConflictReport::NoConflict: return QObject::tr("无冲突");
		break;
	case IntervalConflictReport::LeftConflict: return QObject::tr("左冲突");
		break;
	case IntervalConflictReport::RightConflict: return QObject::tr("右冲突");
		break;
	case IntervalConflictReport::CoverConflict: return QObject::tr("共线冲突");
		break;
	default:return "ERROR Conflict type";
		break;
	}
}
