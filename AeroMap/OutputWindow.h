#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <QtGui>
#include <QGridLayout>
#include <QPlainTextEdit>
#include <QTabWidget>

class OutputWindow : public QWidget
{
	Q_OBJECT

public:
	explicit OutputWindow(QWidget* parent = nullptr);
	~OutputWindow();

	void AppendText(const char* text);

public:

	QGridLayout* mp_gridLayout;
	QGridLayout* mp_gridLayout2;

	QTabWidget* mp_tabWidget;
	QWidget* mp_tabOutput;
	QPlainTextEdit *txtOutput;
	QWidget* mp_tabOther;
};

#endif // #ifndef OUTPUTWINDOW_H

