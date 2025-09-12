#ifndef DRONEPROC_H
#define DRONEPROC_H

#include <QObject>
#include <QThread>

#include "XString.h"
#include "Calc.h"
#include "Gis.h"		// GIS support class

// Stages
#include "StageDataset.h"		// load dataset
#include "StageSFM.h"			// structure from motion
#include "StageMVS.h"			// multi view stereo
#include "StageFilter.h"		// filter point cloud
#include "StageMesh.h"			// create meshes
#include "StageTexture.h"		// apply textures
#include "StageGeoref.h"		// georeference point cloud
#include "StageDEM.h"			// generate DTM/DSM
#include "StageOrtho.h"			// generate orthophoto
#include "StageReport.h"		// generate report

class DroneProc : public QObject
{
    Q_OBJECT

public:

	DroneProc(QObject* parent = nullptr);
	~DroneProc();

	int Run(Stage::Id initStage);

	Stage::Id GetInitStage();

private:

	StageDataset* mp_StageDataset;
	StageSFM* mp_StageSFM;
	StageMVS* mp_StageMVS;
	StageFilter* mp_StageFilter;
	StageMesh* mp_StageMesh;
	StageTexture* mp_StageTexture;
	StageGeoref* mp_StageGeoref;
	StageDEM* mp_StageDEM;
	StageOrtho* mp_StageOrtho;
	StageReport* mp_StageReport;

	Stage::Id m_init_stage;			// start at this stage

//public slots:
//	void RebuildIndex();
//signals:
//	void fileIndexed(QString path);
//	void finished(unsigned int fileCount);
};

#endif // #ifndef DRONEPROC_H
