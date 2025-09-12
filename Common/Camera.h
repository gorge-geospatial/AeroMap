#ifndef CAMERA_H
#define CAMERA_H

#include "MarkTypes.h"
#include "Calc.h"

class Camera
{
public:

	Camera();
	virtual ~Camera();

	VEC3 GetPos() { return mv_Pos; }
	void SetPos(VEC3 pos) { mv_Pos = pos; }
	void SetPos(double x, double y, double z) { mv_Pos.x = x; mv_Pos.y = y; mv_Pos.z = z; }
	void SetX(double x) { mv_Pos.x = x; }
	void SetY(double y) { mv_Pos.y = y; }
	void SetZ(double z) { mv_Pos.z = z; }
	VEC3 GetDir() { return mv_Dir; }
	VEC3 GetUp() { return mv_Up; }
	void SetDirUp(VEC3 dir, VEC3 up);	// since they're so tightly correlated, set both at same time

	// set parameters that define view frustum
	void SetFrustum(double fov, double aspect, double nearDist, double farDist);
	bool PointInFrustum(VEC3& p);

	void SetViewPort(UInt16 cx, UInt16 cy) { m_ViewPort.cx = cx; m_ViewPort.cy = cy; }
	SizeType GetViewPort() { return m_ViewPort; }

	void Pan(double radians);			// pan left/right
	void Tilt(double radians);			// tilt up/down
	void Translate(double dx, double dy, double dz);
	double GetTilt();					// get angle with horizontal plane

	// rotate camera about axis
	void Rotate(double radians);			// about it's own DIR axis
	void RotateAboutX(double radians);		// about world x
	void RotateAboutY(double radians);
	void RotateAboutZ(double radians);

	// move backward/forward along camera's DIR vector
	void MoveBackward(double d);
	void MoveForward(double d);

	// move left/right along camera's DIR x UP vector
	void MoveLeft(double d);
	void MoveRight(double d);

	// move up/down along camera's UP vector
	void MoveUp(double d);
	void MoveDown(double d);

	// move along axes
	void MoveX(double d);
	void MoveY(double d);
	void MoveZ(double d);

	// bound camera movement
	void SetBounds(const VEC3& min, const VEC3& max);
	void ClearBounds();

private:

    VEC3 mv_Pos;
    VEC3 mv_Dir;
    VEC3 mv_Up;

	// size of viewport camera is projecting on
	SizeType m_ViewPort;

	bool mb_IsBounded;
	VEC3 mv_BoundMin;
	VEC3 mv_BoundMax;

	struct Frustum
	{
		bool isValid;		// have valid frustum settings
		double fov;			// field of view, radians
		double aspect;		// aspect ratio
		double nearDist;	// distance to near plane
		double farDist;		// distance to far plane

		Frustum()
		{
			isValid = false;
		}
	};
	Frustum m_Frustum;

	// planes defining the view frustum

	PlaneNP m_planeNear;
	PlaneNP m_planeFar;
	PlaneNP m_planeTop;
	PlaneNP m_planeBottom;
	PlaneNP m_planeLeft;
	PlaneNP m_planeRight;

private:

	bool IsValid(const VEC3& pos);
};

#endif // #ifndef CAMERA_H
