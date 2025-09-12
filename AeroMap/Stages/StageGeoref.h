#ifndef STAGEGEOREF_H
#define STAGEGEOREF_H

#include "Stage.h"

class StageGeoref : Stage
{
public:

	virtual int Run() override;

	StageGeoref(DroneProc* pDroneProc);
	~StageGeoref();
};

#endif // #ifndef STAGEGEOREF_H
