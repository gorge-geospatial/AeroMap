#ifndef STAGEDATASET_H
#define STAGEDATASET_H

#include "AeroMap.h"		// application header
#include "Stage.h"			// base class

class StageDataset : Stage
{
public:

	virtual int Run() override;

	StageDataset(DroneProc* pDroneProc);
	~StageDataset();

private:
	
	int WriteImageListText();
	int WriteImageListJson();

	int InitGeoref();
};

#endif // #ifndef STAGEDATASET_H
