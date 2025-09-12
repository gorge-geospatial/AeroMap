// ElevationDlg.cpp
// Input parameters for applying fractals to terrain.
//

#include "ElevationDlg.h"

#include <QtGui>
#include <QMessageBox>

ElevationDlg::ElevationDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	// make dialog fixed size
	setFixedSize(size());
	// remove question mark from title bar
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	// signals and slots
	verify_connect(cmdClose, SIGNAL(clicked()), this, SLOT(OnClose()));
	verify_connect(cmdApply, SIGNAL(clicked()), this, SLOT(OnApply()));
	verify_connect(optDelta, SIGNAL(clicked()), this, SLOT(OnOptDeltaClicked()));
	verify_connect(optScale, SIGNAL(clicked()), this, SLOT(OnOptScaleClicked()));
	verify_connect(optSmooth, SIGNAL(clicked()), this, SLOT(OnOptSmoothClicked()));
}

ElevationDlg::~ElevationDlg()
{
}

void ElevationDlg::OnOptDeltaClicked()
{
	spinDelta->setEnabled(true);
	spinScale->setEnabled(false);
}

void ElevationDlg::OnOptScaleClicked()
{
	spinDelta->setEnabled(false);
	spinScale->setEnabled(true);
}

void ElevationDlg::OnOptSmoothClicked()
{
	spinDelta->setEnabled(false);
	spinScale->setEnabled(false);
}

void ElevationDlg::OnApply()
{
	accept();
}

void ElevationDlg::OnClose()
{
	reject();
}
