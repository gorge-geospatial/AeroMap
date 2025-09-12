// Material.cpp
// Material manager
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "Calc.h"
#include "Material.h"

Material::Material()
	: m_Tr(0.0F)
	, m_Tf(1.0F, 1.0F, 1.0F)
{
	m_Ambient.r = m_Ambient.g = m_Ambient.b = 0.4F;
	m_Diffuse.r = m_Diffuse.g = m_Diffuse.b = 0.3F;
	m_Specular.r = m_Specular.g = m_Specular.b = 0.1F;

	// shininess specifies how small the highlights are: 
	// the shinier, the smaller the highlights. 
	m_Shininess = 100.0f;
}

Material::~Material()
{
}

void Material::Clear()
{
	ms_MatName.Clear();

	m_Ambient = glm::vec3(0.0F, 0.0F, 0.0F);
	m_Diffuse = glm::vec3(0.0F, 0.0F, 0.0F);
	m_Specular = glm::vec3(0.0F, 0.0F, 0.0F);

	m_Shininess = 0;
	m_Tr = 0.0F;
	m_Tf = glm::vec3(1.0F, 1.0F, 1.0F);
	m_illum = 1;

	m_map_Ka.Clear();
	m_map_Kd.Clear();
	m_map_Ks.Clear();
	m_map_Ns.Clear();
	m_map_d.Clear();
	m_map_bump.Clear();
	m_map_disp.Clear();
	m_map_decal.Clear();
}

void Material::SetColor(const glm::vec3& ad)
{
	// Set ambient & diffuse to same color.
	//

	SetAmbient(ad);
	SetDiffuse(ad);
}

void Material::SetAmbient(const glm::vec3& ambient)
{
	m_Ambient.r = Clamp(ambient.r, 0.0F, 1.0F);
	m_Ambient.g = Clamp(ambient.g, 0.0F, 1.0F);
	m_Ambient.b = Clamp(ambient.b, 0.0F, 1.0F);
}

glm::vec3 Material::GetAmbient()
{
	return m_Ambient;
}

void Material::SetDiffuse(const glm::vec3& diffuse)
{
	m_Diffuse.r = Clamp(diffuse.r, 0.0F, 1.0F);
	m_Diffuse.g = Clamp(diffuse.g, 0.0F, 1.0F);
	m_Diffuse.b = Clamp(diffuse.b, 0.0F, 1.0F);
}

glm::vec3 Material::GetDiffuse()
{
	return m_Diffuse;
}

void Material::SetSpecular(const glm::vec3& specular)
{
	m_Specular.r = Clamp(specular.r, 0.0F, 1.0F);
	m_Specular.g = Clamp(specular.g, 0.0F, 1.0F);
	m_Specular.b = Clamp(specular.b, 0.0F, 1.0F);
}

glm::vec3 Material::GetSpecular()
{
	return m_Specular;
}

void Material::SetShininess(float shininess)
{
	m_Shininess = shininess;
}

float Material::GetShininess()
{
	return m_Shininess;
}

void Material::SetTransparency(float tr)
{
	m_Tr = tr;
}

float Material::GetTransparency()
{
	return m_Tr;
}

void Material::SetTransmission(const glm::vec3& tf)
{
	m_Tf = tf;
}

glm::vec3 Material::GetTransmission()
{
	return m_Tf;
}

void Material::SetIllum(UInt16 illum)
{
	m_illum = illum;
}

UInt16 Material::GetIllum()
{
	return m_illum;
}

void Material::SetMapAmbient(XString mapKa)
{
	// ambient texture map
	m_map_Ka = mapKa;
}

XString Material::GetMapAmbient()
{
	return m_map_Ka;
}

void Material::SetMapDiffuse(XString mapKd)
{
	// diffuse texture map
	m_map_Kd = mapKd;
}

XString Material::GetMapDiffuse()
{
	return m_map_Kd;
}

void Material::SetMapSpecular(XString mapKs)
{
	// specular color texture map
	m_map_Ks = mapKs;
}

XString Material::GetMapSpecular()
{
	return m_map_Ks;
}

void Material::SetMapNs(XString mapNs)
{
	// specular highlight component
	m_map_Ns = mapNs;
}

XString Material::GetMapNs()
{
	return m_map_Ns;
}

void Material::SetMapAlpha(XString mapAlpha)
{
	// alpha texture map
	m_map_d = mapAlpha;
}

XString Material::GetMapAlpha()
{
	return m_map_d;
}

void Material::SetMapBump(XString mapBump)
{
	// bump map (which by default uses luminance channel of the image)
	m_map_bump = mapBump;
}

XString Material::GetMapBump()
{
	return m_map_bump;
}

void Material::SetMapDisp(XString mapDisp)
{
	// displacement map
	m_map_disp = mapDisp;
}

XString Material::GetMapDisp()
{
	return m_map_disp;
}

void Material::SetMapDecal(XString mapDecal)
{
	// stencil decal texture (defaults to 'matte' channel of the image)
	m_map_decal = mapDecal;
}

XString Material::GetMapDecal()
{
	return m_map_decal;
}

XString  Material::GetName() const
{
	return ms_MatName;
}

void Material::SetName(const char* name)
{
	if (name != nullptr)
		ms_MatName = name;
}
