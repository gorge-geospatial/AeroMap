// MetaDataDlg.cpp
// General purpose data display dialog.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "AeroMap.h"			// application header
#include "MetaDataDlg.h"

#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

MetaDataDlg::MetaDataDlg(QWidget* parent, const char* title)
	: QDialog(parent)
{
	setupUi(this);
	// remove question mark from title bar
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	if (title != nullptr)
		setWindowTitle(title);

	// signals and slots
	verify_connect(cmdClose, SIGNAL(clicked()), this, SLOT(OnClose()));
	verify_connect(cmdClipboard, SIGNAL(clicked()), this, SLOT(OnClipBoard()));
}

MetaDataDlg::~MetaDataDlg()
{
}

void MetaDataDlg::SetMetaData(XString& strMeta)
{
	txtMeta->setPlainText(strMeta.c_str());
}

XString MetaDataDlg::GetMetaData() const
{
	return XString(txtMeta->toPlainText());
}

void MetaDataDlg::OnClipBoard()
{
	QClipboard* pClipboard = QApplication::clipboard();
	if (pClipboard)
	{
		QString originalText = pClipboard->text();
		pClipboard->setText(txtMeta->toPlainText());
	}
}

void MetaDataDlg::OnClose()
{
    reject();
}
