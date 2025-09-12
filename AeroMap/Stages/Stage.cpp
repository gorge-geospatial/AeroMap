// Stage.cpp
// Base class for all stages.
//

#include "DroneProc.h"
#include "Stage.h"

void Stage::BenchmarkStart()
{
	m_Timer.start();
}

void Stage::BenchmarkStop(bool init)
{
	double sec = m_Timer.elapsed() * 0.001;

	XString file_name = XString::CombinePath(GetProject().GetDroneOutputPath(), "benchmark.txt");

	FILE* pFile = nullptr;
	if (init)
		pFile = fopen(file_name.c_str(), "wt");
	else
		pFile = fopen(file_name.c_str(), "at");

	if (pFile)
	{
		fprintf(pFile, "%s: %0.3f\n", Stage::IdToText(m_stage_id), sec);
		fclose(pFile);
	}
}

void Stage::LogStage()
{
	// Write stage to run log.
	//

	AeroLib::LogWrite("========================================");
	AeroLib::LogWrite("Stage: %s", Stage::IdToText(m_stage_id));
	AeroLib::LogWrite("========================================");
}

bool Stage::Rerun()
{
	// Force stage to run regardless of output
	// file state.
	//

	bool rerun = (m_stage_id >= mp_DroneProc->GetInitStage());
	return rerun;
}

const char* Stage::IdToText(Id stage_id)
{
	switch(stage_id) {
	case Dataset: return "Load Dataset";
	case OpenSFM: return "OpenSFM";
	case OpenMVS: return "OpenMVS";
	case Filter: return "Filter Points";
	case Mesh: return "Mesh";
	case Texture: return "Texture";
	case Georeference: return "Georeference";
	case DEM: return "DEM";
	case Orthophoto: return "Orthophoto";
	case Report: return "Report";
	}

	return "---";
}

Stage::Stage(DroneProc* pDroneProc)
	: mp_DroneProc(pDroneProc)
{

}

Stage::~Stage()
{

}
