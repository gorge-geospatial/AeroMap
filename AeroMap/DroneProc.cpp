// DroneProc.cpp
// Drone photogrammetry pipeline manager.
//

#include <stdio.h>
#include <assert.h>

#include <QDir>
#include <QString>

#include "AeroMap.h"
#include "Logger.h"			// message logging facility
#include "Calc.h"
#include "DroneProc.h"

DroneProc::DroneProc(QObject* parent)
	: QObject(parent)
	, mp_StageDataset(new StageDataset(this))
	, mp_StageSFM(new StageSFM(this))
	, mp_StageMVS(new StageMVS(this))
	, mp_StageFilter(new StageFilter(this))
	, mp_StageMesh(new StageMesh(this))
	, mp_StageTexture(new StageTexture(this))
	, mp_StageGeoref(new StageGeoref(this))
	, mp_StageDEM(new StageDEM(this))
	, mp_StageOrtho(new StageOrtho(this))
	, mp_StageReport(new StageReport(this))
{
}

DroneProc::~DroneProc()
{
	delete mp_StageDataset;
	delete mp_StageSFM;
	delete mp_StageMVS;
	delete mp_StageFilter;
	delete mp_StageMesh;
	delete mp_StageTexture;
	delete mp_StageGeoref;
	delete mp_StageDEM;
	delete mp_StageOrtho;
	delete mp_StageReport;
}

int DroneProc::Run(Stage::Id initStage)
{
	int status = 0;
	
	m_init_stage = initStage;

	AeroLib::CreateFolder(GetProject().GetDroneOutputPath());
	AeroLib::InitRunLog();
	GetProject().PreProcessArgs();

	int load_status = GetProject().LoadImageList();
	if (load_status == 0)
	{
		GetApp()->LogWrite("No images found.");
		return -1;
	}
	else if (load_status == -1)
	{
		GetApp()->LogWrite("Canceled.");
		return -1;
	}

	status = mp_StageDataset->Run();

	if (status == 0)
		status = mp_StageSFM->Run();

	if (status == 0)
		status = mp_StageMVS->Run();

	if (status == 0)
		status = mp_StageFilter->Run();

	if (status == 0)
		status = mp_StageMesh->Run();

	if (status == 0)
		status = mp_StageTexture->Run();

	if (status == 0)
		status = mp_StageGeoref->Run();

	if (status == 0)
		status = mp_StageDEM->Run();

	if (status == 0)
		status = mp_StageOrtho->Run();

	if (status == 0)
		status = mp_StageReport->Run();

	GetApp()->LogWrite("Drone image processing complete.");

	return status;
}

Stage::Id DroneProc::GetInitStage()
{
	return m_init_stage;
}
