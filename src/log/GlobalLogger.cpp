#include "GlobalLogger.h"
#include <QTextBrowser>
#include <iostream>
#include "util/utilfunc.h"

std::unique_ptr<GlobalLogger> GlobalLogger::_instance;

void GlobalLogger::setOutWidget(QTextBrowser* w)
{
	text_out = w;
}

QTextBrowser* GlobalLogger::outWidget() const
{
	return text_out;
}

void GlobalLogger::handleLog(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	auto txt = formatMsg(type, context, msg);
	if (text_out) {
		text_out->append(txt);
	}
	else {
#ifdef _DEBUG
		auto b = msg.toLocal8Bit();
		std::printf(b.constData());
#endif
	}
}

GlobalLogger* GlobalLogger::get()
{
	if (!_instance) {
		_instance.reset(new GlobalLogger);
	}
	return _instance.get();
}

QString GlobalLogger::formatMsg(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	return QString("[%1] %2%3").arg(
		qeutil::msgTypeToString(type),
		GlobalLogger::msgLocation(type, context),
		msg
	);
}

QString GlobalLogger::msgLocation(QtMsgType type, const QMessageLogContext& context)
{
	switch (type)
	{
	case QtWarningMsg: 
	case QtCriticalMsg: 
	case QtFatalMsg: 
		return QString("%1:%2").arg(
			context.file ? context.file : "",
			context.function ? context.function : ""
		);
	default:
		return "";
	}
}

void qeutil::qeLogHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	GlobalLogger::get()->handleLog(type, context, msg);
}
