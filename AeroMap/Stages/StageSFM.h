#ifndef STAGESFM_H
#define STAGESFM_H

#include "AeroMap.h"		// application header
#include "exif.h"			// easy exif header
#include "Stage.h"			// base class

class StageSFM : Stage
{
public:

	virtual int Run() override;

	StageSFM(DroneProc* pDroneProc);
	~StageSFM();

private:
	
	int Setup();

	void NewSfmPipeline();

	void DetectFeatures();
	void MatchFeatures();
	void CreateTracks();
	void Reconstruct();
	void ComputeStatistics();
	void ExportGeocoords();
	void Undistort();
	void ExportVisualSFM();
	void ExportPly();

	int WriteExif();
	int WriteImageListText();
	int WriteCameraModelsJson();
	int WriteReferenceLLA();
	int WriteConfigYaml();

	int WriteCamerasJson();
};

#endif //#ifndef STAGESFM_H
