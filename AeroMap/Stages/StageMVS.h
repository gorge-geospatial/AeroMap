#ifndef STAGEMVS_H
#define STAGEMVS_H

#include "Stage.h"

class StageMVS : Stage
{
public:

	virtual int Run() override;

	StageMVS(DroneProc* pDroneProc);
	~StageMVS();
};

#endif // #ifndef STAGEMVS_H
