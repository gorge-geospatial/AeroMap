#ifndef STAGE_H
#define STAGE_H

#include <QElapsedTimer>

#include "AeroMap.h"
#include "AeroLib.h"

class DroneProc;

class Stage
{
public:

	enum Id				// ordered list of stages
	{
		Dataset,
		OpenSFM,
		OpenMVS,
		Filter,
		Mesh,
		Texture,
		Georeference,
		DEM,
		Orthophoto,
		Report
	};

public:

	Stage(DroneProc* pDroneProc);
	~Stage();

	virtual int Run() = 0;

	static const char* IdToText(Id stage_id);

protected:

	Id m_stage_id;					// this stage's id
	DroneProc* mp_DroneProc;		// parent drone proc instance

protected:

	void BenchmarkStart();
	void BenchmarkStop(bool init = false);
	void LogStage();

	bool Rerun();

private:

	QElapsedTimer m_Timer;
};

#endif // #ifndef STAGE_H
