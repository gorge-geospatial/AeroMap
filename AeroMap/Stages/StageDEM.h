#ifndef STAGEDEM_H
#define STAGEDEM_H

#include "Stage.h"

class StageDEM : Stage
{
public:

	virtual int Run() override;

	StageDEM(DroneProc* pDroneProc);
	~StageDEM();

private:

	void CreateTerrainModel(XString model_type);
};

#endif // #ifndef STAGEDEM_H
