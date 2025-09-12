#ifndef STAGEORTHO_H
#define STAGEORTHO_H

#include "Stage.h"

class StageOrtho : Stage
{
public:

	virtual int Run() override;

	StageOrtho(DroneProc* pDroneProc);
	~StageOrtho();
};

#endif // #ifndef STAGEORTHO_H
