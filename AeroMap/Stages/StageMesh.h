#ifndef STAGEMESH_H
#define STAGEMESH_H

#include "Stage.h"

class StageMesh : Stage
{
public:

	virtual int Run() override;

	StageMesh(DroneProc* pDroneProc);
	~StageMesh();

private:

	int CreateMesh3D();
	int CreateMesh25D();
};

#endif // #ifndef STAGEMESH_H
