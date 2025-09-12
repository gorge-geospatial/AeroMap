// LidarModelDlg.cpp
// Lidar DTM/DSM options.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "AeroMap.h"			// application header
#include "LidarModelDlg.h"

#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

LidarModelDlg::LidarModelDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	// make dialog fixed size
	setFixedSize(size());
	// remove question mark from title bar
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	cmdCreate->setEnabled(false);

	// load data
	QSettings settings(ORG_KEY, APP_KEY);
	txtSourceFile->setText(settings.value("LidarModelDlg.SourceFile", "").toString());
	txtOutputFolder->setText(settings.value("LidarModelDlg.OutputPath", "").toString());

	// signals and slots
	verify_connect(cmdCreate, SIGNAL(clicked()), this, SLOT(OnCreate()));
	verify_connect(cmdCancel, SIGNAL(clicked()), this, SLOT(OnClose()));
	verify_connect(cmdSourceFile, SIGNAL(clicked()), this, SLOT(OnSelectSourceFile()));
	verify_connect(cmdOutputFolder, SIGNAL(clicked()), this, SLOT(OnSelectOutputPath()));
	verify_connect(chkDTM, SIGNAL(clicked()), this, SLOT(OnDtmClicked()));
	verify_connect(chkDSM, SIGNAL(clicked()), this, SLOT(OnDsmClicked()));
}

LidarModelDlg::~LidarModelDlg()
{
}

void LidarModelDlg::OnCreate()
{
    // save data
	QSettings settings(ORG_KEY, APP_KEY);
	settings.setValue("LidarModelDlg.SourceFile", txtSourceFile->text());
	settings.setValue("LidarModelDlg.OutputPath", txtOutputFolder->text());

    accept();
}

void LidarModelDlg::OnClose()
{
    reject();
}

void LidarModelDlg::EnableCreate()
{
	// must specify at least one output type
	cmdCreate->setEnabled(false);
	if (chkDTM->isChecked())
		cmdCreate->setEnabled(true);
	if (chkDSM->isChecked())
		cmdCreate->setEnabled(true);
}

void LidarModelDlg::OnDtmClicked()
{
	EnableCreate();
}

void LidarModelDlg::OnDsmClicked()
{
	EnableCreate();
}

void LidarModelDlg::SetSourceFile(const char* fileName)
{
	XString strSourceFile = fileName;
	strSourceFile.Trim();

	txtSourceFile->setText(strSourceFile.c_str());

	// default output folder to location of input file
	if (txtOutputFolder->text().length() == 0)
		txtOutputFolder->setText(strSourceFile.GetPathName().c_str());
}

XString LidarModelDlg::GetSourceFile()
{
	return XString(txtSourceFile->text());
}

XString LidarModelDlg::GetOutputFolder()
{
	return XString(txtOutputFolder->text());
}

double LidarModelDlg::GetResolution()
{
	return spinResolution->value();
}

bool LidarModelDlg::IsDsmSelected()
{
	return chkDSM->isChecked();
}

bool LidarModelDlg::IsDtmSelected()
{
	return chkDTM->isChecked();
}

void LidarModelDlg::OnSelectSourceFile()
{
	XString defaultPath = "";

	XString fileName = QFileDialog::getOpenFileName(
		this,
		tr("Select Source File"),
		defaultPath.c_str(),
		tr("Las Files (*.las);;Laz Files (*.laz)"));

	if (fileName.GetLength() > 0)
	{
		SetSourceFile(fileName.c_str());
	}
}

void LidarModelDlg::OnSelectOutputPath()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select Folder"),
		"/",
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.length() > 0)
	{
		txtOutputFolder->setText(dir);
	}
}
