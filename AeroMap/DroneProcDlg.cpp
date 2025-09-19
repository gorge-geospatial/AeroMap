// DroneProcDlg.cpp
// Drone photogrammetry options.
//

#include "AeroMap.h"			// application header
#include "DroneProcDlg.h"

#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

DroneProcDlg::DroneProcDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	// make dialog fixed size
	setFixedSize(size());
	// remove question mark from title bar
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	LoadComboBoxes();

	// load defaults
	spinDemResolution->setValue(arg.dem_resolution);
	spinDemGapFillSteps->setValue(arg.dem_gapfill_steps);
	spinDemDecimation->setValue(arg.dem_decimation);
	spinOrthoResolution->setValue(arg.ortho_resolution);
	spinCrop->setValue(arg.crop);

	cboPointCloudQuality->setCurrentText(arg.pc_quality.c_str());
	spinPointCloudFilter->setValue(arg.pc_filter);
	spinPointCloudSample->setValue(arg.pc_sample);

	cboFeatureType->setCurrentText(arg.feature_type.c_str());
	spinSiftEdgeThreshold->setValue(arg.sift_edge_threshold);
	spinSiftPeakThreshold->setValue(arg.sift_peak_threshold);

	chkDevSFM->setChecked(arg.dev_sfm);

	chkDSM->setChecked(arg.dsm);
	chkDTM->setChecked(arg.dtm);

	tabWidget->setCurrentIndex(0);

	// signals and slots
	verify_connect(cmdRun, SIGNAL(clicked()), this, SLOT(OnRun()));
	verify_connect(cmdCancel, SIGNAL(clicked()), this, SLOT(OnClose()));
}

DroneProcDlg::~DroneProcDlg()
{
}

void DroneProcDlg::OnRun()
{
	arg.dem_resolution = spinDemResolution->value();
	arg.dem_gapfill_steps = spinDemGapFillSteps->value();
	arg.dem_decimation = spinDemDecimation->value();
	arg.ortho_resolution = spinOrthoResolution->value();
	arg.crop = spinCrop->value();

	arg.pc_quality = cboPointCloudQuality->currentText();
	arg.pc_filter = spinPointCloudFilter->value();
	arg.pc_sample = spinPointCloudSample->value();

	arg.feature_type = cboFeatureType->currentText();
	arg.sift_edge_threshold = spinSiftEdgeThreshold->value();
	arg.sift_peak_threshold = spinSiftPeakThreshold->value();

	arg.dev_sfm = chkDevSFM->isChecked();

	arg.dsm = chkDSM->isChecked();
	arg.dtm = chkDTM->isChecked();

    accept();
}

void DroneProcDlg::OnClose()
{
    reject();
}

Stage::Id DroneProcDlg::GetInitStage()
{
	return (Stage::Id)cboInitStage->currentData().toInt();
}

void DroneProcDlg::LoadComboBoxes()
{
	cboInitStage->clear();
	cboInitStage->addItem("Load Dataset", Stage::Id::Dataset);
	cboInitStage->addItem("OpenSFM", Stage::Id::OpenSFM);
	cboInitStage->addItem("OpenMVS", Stage::Id::OpenMVS);
	cboInitStage->addItem("Filter Points", Stage::Id::Filter);
	cboInitStage->addItem("Mesh", Stage::Id::Mesh);
	cboInitStage->addItem("Texture", Stage::Id::Texture);
	cboInitStage->addItem("Georeference", Stage::Id::Georeference);
	cboInitStage->addItem("DEM", Stage::Id::DEM);
	cboInitStage->addItem("Orthophoto", Stage::Id::Orthophoto);
	cboInitStage->addItem("Report", Stage::Id::Report);

	cboPointCloudQuality->clear();
	cboPointCloudQuality->addItem("ultra");
	cboPointCloudQuality->addItem("high");
	cboPointCloudQuality->addItem("medium");
	cboPointCloudQuality->addItem("low");
	cboPointCloudQuality->addItem("lowest");

	cboFeatureType->clear();
	cboFeatureType->addItem("SIFT");
	//akaze, hahog, orb, sift
}
