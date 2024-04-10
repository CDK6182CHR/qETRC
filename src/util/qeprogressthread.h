#pragma once

#include <QThread>
#include <functional>

class QProgressDialog;
class QWidget;

/**
 * 2024.04.10  This is a QThread combined with QProgressDialog.
 * The thread is created with a callable object (usually lambda) as similar to QThread::createThread, 
 * but the functor should receive a pointer to this, for some operations like updating the progress status.
 */
class QEProgressThread : public QThread
{
	Q_OBJECT;

#if __cpp_lib_move_only_function >= 202110L
	using functor_t = std::move_only_function<void(QEProgressThread*> );
#else
	using functor_t = std::function<void(QEProgressThread*)>;
#endif

	QProgressDialog* dlg;
	functor_t func;

public:
	QEProgressThread(functor_t&& func, QWidget* parent = nullptr);
	auto* progressDialog() { return dlg; }

public slots:
	// encapsule some functions. Note these functions are designed to be called inside run() with QMetaObject::invokeMethod.
	void setLabelText(const QString& text);
	void setValue(int value);

private slots:
	//void onFinished();
};
