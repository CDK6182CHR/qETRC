#pragma once

#include <QString>

namespace qeutil {

	/**
	 * 2025.08.17  Get the full path of a system file (config.json, system.json, etc).
	 * Currently, we always return the path for the installation directory.
	 */
	QString getSystemFileFullPath(const QString& filename);
}