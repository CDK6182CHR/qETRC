#include "filepaths.h"

#include <QCoreApplication>

QString qeutil::getSystemFileFullPath(const QString& filename)
{
	return QCoreApplication::applicationDirPath() + '/' + filename;
}
