#include "qeprogressthread.h"

#include <QProgressDialog>

QEProgressThread::QEProgressThread(functor_t&& func, QWidget* parent):
	QThread(parent), func(std::forward<functor_t>(func))
{
	// init dialog
	dlg = new QProgressDialog(parent);   // The other info should be set outside

	// Automatically clean-up on finished
	connect(this, &QThread::finished, [this]() {
		dlg->reset();
		dlg->deleteLater();
		//this->deleteLater();  // not called here; use it outside
		});
}

void QEProgressThread::run()
{
	_ret = func(this);
	exit(_ret);
}

void QEProgressThread::setLabelText(const QString& txt)
{
	QMetaObject::invokeMethod(dlg, "setLabelText", txt);
}

void QEProgressThread::setValue(int value)
{
	QMetaObject::invokeMethod(dlg, "setValue", value);
}

