// Light.cpp
// Light manager
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "Light.h"

Light::Light(Type lightType)
	: m_LightType(lightType)
	, m_Position(0, 0, 0)
	, m_Direction(0, 1, 0)
	, m_Color(1, 1, 1)
	, mb_Enable(true)
	, m_Ambient(1.0F)
	, m_Diffuse(1.0F)
	, m_Specular(1.0F)
{
	if (IsDirectional() || IsSpot())
	{
	}

	if (!IsDirectional())
	{
	}
	
	if (IsSpot())
	{
		// Decrease in illumination between a spotlight's inner cone (the angle specified by Theta) and the outer edge of the outer cone 
		// (the angle specified by Phi).
		// The effect of falloff on the lighting is subtle. Furthermore, a small performance penalty is incurred by shaping the falloff curve. 
		// For these reasons, most developers set this value to 1.0.
		//	m_Light.Falloff = 1.0;
	
		//Angle, in radians, of a spotlight's inner cone - that is, the fully illuminated spotlight cone. This value must be in the range from 0 through the value specified by Phi.
		//	m_Light.Theta = 0;

		//Angle, in radians, defining the outer edge of the spotlight's outer cone. Points outside this cone are not lit by the spotlight. This value must be between 0 and pi. 
		//	m_Light.Phi = 0.0;
	}
}

Light::~Light()
{
}

void Light::SetPos(VEC3& pos)
{
	m_Position = pos;
}

void Light::SetPos(double x, double y, double z)
{
	m_Position.x = x;
	m_Position.y = y;
	m_Position.z = z;
}

void Light::SetDir(VEC3& dir)
{
	m_Direction = dir;
}

VEC3 Light::GetPos()
{
	return m_Position;
}

VEC3 Light::GetDir()
{
	return m_Direction;
}

void Light::SetColor(float r, float g, float b)
{
	m_Color.x = Clamp(r, 0.0F, 1.0F);
	m_Color.y = Clamp(g, 0.0F, 1.0F);
	m_Color.z = Clamp(b, 0.0F, 1.0F);
}

float Light::GetRed()
{
	return static_cast<float>(m_Color.x);
}

float Light::GetGreen()
{
	return static_cast<float>(m_Color.y);
}

float Light::GetBlue()
{
	return static_cast<float>(m_Color.z);
}

float Light::GetAmbient()
{
	// Ambient, diffuse & specular implemented
	// as single intensity value. A more complete
	// description would be for each to b RGB.

	return m_Ambient;
}

float Light::GetDiffuse()
{
	// Ambient, diffuse & specular implemented
	// as single intensity value. A more complete
	// description would be for each to b RGB.

	return m_Diffuse;
}

float Light::GetSpecular()
{
	// Ambient, diffuse & specular implemented
	// as single intensity value. A more complete
	// description would be for each to b RGB.

	return m_Specular;
}

void Light::SetAmbient(float ambient)
{
	m_Ambient = Clamp(ambient, 0.0F, 1.0F);
}

void Light::SetDiffuse(float diffuse)
{
	m_Diffuse = Clamp(diffuse, 0.0F, 1.0F);
}

void Light::SetSpecular(float specular)
{
	m_Specular = Clamp(specular, 0.0F, 1.0F);
}

void Light::RotateAboutX(double radians)
{
	RotateX(radians, m_Position);
//TODO:
	//if rotatedir
	//	RoateX(radisn, m_Dir);
}

void Light::RotateAboutY(double radians)
{
	RotateY(radians, m_Position);
}

void Light::RotateAboutZ(double radians)
{
	RotateZ(radians, m_Position);
}

void Light::TurnOn()
{
	mb_Enable = true;
}

void Light::TurnOff()
{
	mb_Enable = false;
}

