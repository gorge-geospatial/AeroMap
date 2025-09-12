#ifndef STAGEFILTER_H
#define STAGEFILTER_H

#include "Stage.h"

class StageFilter : Stage
{
public:

	virtual int Run() override;

	StageFilter(DroneProc* pDroneProc);
	~StageFilter();
};

#endif // #ifndef STAGEFILTER_H
