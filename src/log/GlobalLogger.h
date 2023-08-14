#pragma once
#include <QtLogging>
#include <memory>


class QTextBrowser;

/**
 * Global log handler helpers with Qt logging system.
 * This class is global singleton and provides necessary info for logging.
 * Maintains File handles (if required, leaving for future)
 */
class GlobalLogger {
	
public:
	void setOutWidget(QTextBrowser* w);
	QTextBrowser* outWidget()const;

	void handleLog(QtMsgType type, const QMessageLogContext& context, const QString& msg);

	static GlobalLogger* get();

private:
	QTextBrowser* text_out = nullptr;
	static std::unique_ptr<GlobalLogger> _instance;

	QString formatMsg(QtMsgType type, const QMessageLogContext& context, const QString& msg);
	static QString msgLocation(QtMsgType type, const QMessageLogContext& context);
};


namespace qeutil {
	void qeLogHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
}
