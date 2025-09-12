// OutputWindow.cpp
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "AeroMap.h"
#include "OutputWindow.h"

OutputWindow::OutputWindow(QWidget* parent)
	: QWidget(parent)
	, mp_gridLayout(nullptr)
	, mp_gridLayout2(nullptr)
	, mp_tabOther(nullptr)
	, mp_tabOutput(nullptr)
	, mp_tabWidget(nullptr)
{
	this->resize(700, 120);

	mp_gridLayout = new QGridLayout(this);
	mp_tabWidget = new QTabWidget(this);
	mp_tabOutput = new QWidget();
	mp_gridLayout2 = new QGridLayout(mp_tabOutput);

	QFont font("Courier New", 10.0);
	txtOutput = new QPlainTextEdit(mp_tabOutput);
	txtOutput->setFont(font);

	mp_gridLayout2->addWidget(txtOutput, 0, 0, 1, 1);

	mp_tabWidget->addTab(mp_tabOutput, QString());

	mp_tabOther = new QWidget();
	//mp_tabWidget->addTab( tabOther, QString() );
	mp_gridLayout->addWidget(mp_tabWidget, 0, 0, 1, 1);

	mp_tabWidget->setTabText(0, "Messages");
	mp_tabWidget->setTabText(1, "");

	//retranslateUi(outputWidget);

	mp_tabWidget->setCurrentIndex(0);

	QMetaObject::connectSlotsByName(this);
}

OutputWindow::~OutputWindow()
{
	delete mp_gridLayout;
	delete mp_gridLayout2;

	delete mp_tabOther;
	delete mp_tabOutput;
	delete mp_tabWidget;
}

void OutputWindow::AppendText(const char* text)
{
	txtOutput->appendPlainText(text);
	GetApp()->processEvents();
}
