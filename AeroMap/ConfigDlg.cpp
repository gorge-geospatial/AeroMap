// ConfigDlg.cpp
// Application configuration settings.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "AeroMap.h"			// application header
#include "ConfigDlg.h"

#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

ConfigDlg::ConfigDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	// make dialog fixed size
	setFixedSize(size());
	// remove question mark from title bar
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	cboStartup->clear();
	cboStartup->addItem("Show Empty Workspace", 0);
	cboStartup->addItem("Load Last Loaded Project", 1);

	// load data
	QSettings settings(ORG_KEY, APP_KEY);
	txtAppDataPath->setText(settings.value(PATH_APPDATA_KEY, "").toString());
	int startupState = settings.value(STARTUP_STATE_KEY, 0).toInt();

	for (int i = 0; i < cboStartup->count(); ++i)
	{
		if (cboStartup->itemData(i).toInt() == startupState)
		{
			cboStartup->setCurrentIndex(i);
			break;
		}
	}

	// signals and slots
	verify_connect(cmdClose, SIGNAL(clicked()), this, SLOT(OnClose()));
	verify_connect(cmdSave, SIGNAL(clicked()), this, SLOT(OnSave()));
	verify_connect(cmdAppDataPath, SIGNAL(clicked()), this, SLOT(OnSelectAppDataPath()));
}

ConfigDlg::~ConfigDlg()
{
}

void ConfigDlg::OnSave()
{
    // save data
	QSettings settings(ORG_KEY, APP_KEY);
	settings.setValue(PATH_APPDATA_KEY, txtAppDataPath->text());
	settings.setValue(STARTUP_STATE_KEY, cboStartup->currentData().toInt());

    accept();
}

void ConfigDlg::OnClose()
{
    reject();
}

void ConfigDlg::OnSelectAppDataPath()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select Folder"),
		"/",
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.length() > 0)
	{
		txtAppDataPath->setText(dir);
	}
}

