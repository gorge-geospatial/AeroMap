#ifndef STAGETEXTURE_H
#define STAGETEXTURE_H

#include "Stage.h"

class StageTexture : Stage
{
public:

	virtual int Run() override;

	StageTexture(DroneProc* pDroneProc);
	~StageTexture();

private:

	int m_max_texture_size;

private:
	
	int TextureModel3D();
	int TextureModel25D();
};

#endif // #ifndef STAGETEXTURE_H
