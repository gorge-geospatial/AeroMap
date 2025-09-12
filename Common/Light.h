#ifndef LIGHT_H
#define LIGHT_H

#include "Calc.h"

class Light
{
public:

	enum class Type
	{
		Ambient,
		Directional,
		Point,
		Spot
	};

public:

	Light(Type type = Type::Directional);
	virtual ~Light();

	void SetPos(VEC3& pos);
	void SetPos(double x, double y, double z);
	void SetDir(VEC3& dir);

	VEC3 GetPos();
	VEC3 GetDir();

	void SetColor(float r, float g, float b);
	float GetRed();
	float GetGreen();
	float GetBlue();

	float GetAmbient();
	float GetDiffuse();
	float GetSpecular();
	void  SetAmbient(float ambient);
	void  SetDiffuse(float diffuse);
	void  SetSpecular(float specular);

	void RotateAboutX(double radians);
	void RotateAboutY(double radians);
	void RotateAboutZ(double radians);

	void TurnOn();
	void TurnOff();

	bool IsDirectional() { return m_LightType == Type::Directional; }
	bool IsSpot() { return m_LightType == Type::Point; }
	bool IsPoint() { return m_LightType == Type::Spot; }

private:
	    
	VEC3 m_Position;			// light position
	VEC3 m_Direction;			// light direction
	VEC3 m_Color;				// RGB color

	Type m_LightType;

	bool mb_Enable;				// light on/off switch

	float m_Ambient;			// ambient intensity
	float m_Diffuse;			// diffuse intensity
	float m_Specular;			// specular intensity
};

#endif // #ifndef LIGHT_H
