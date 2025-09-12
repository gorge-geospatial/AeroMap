#ifndef MATERIAL_H
#define MATERIAL_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "XString.h"

class Material
{
public:

	Material();
	virtual ~Material();

	void SetColor(const glm::vec3& ad);		// set ambient & diffuse to same color

	void SetAmbient(const glm::vec3& ambient);
	glm::vec3 GetAmbient();

	void SetDiffuse(const glm::vec3& diffuse);
	glm::vec3 GetDiffuse();

	void SetSpecular(const glm::vec3& specular);
	glm::vec3 GetSpecular();

	void SetShininess(float shininess);
	float GetShininess();

	void SetTransparency(float tr);
	float GetTransparency();

	void SetTransmission(const glm::vec3& tf);
	glm::vec3 GetTransmission();

	XString GetName() const;
	void SetName(const char* name);

	void SetIllum(UInt16 illum);
	UInt16 GetIllum();

	// texture maps
	void SetMapAmbient(XString mapKa);			// ambient texture map
	XString GetMapAmbient();
	void SetMapDiffuse(XString mapKd);			// diffuse texture map
	XString GetMapDiffuse();
	void SetMapSpecular(XString mapKs);			// specular color texture map
	XString GetMapSpecular();
	void SetMapNs(XString mapNs);				// specular highlight component
	XString GetMapNs();
	void SetMapAlpha(XString mapAlpha);			// alpha texture map
	XString GetMapAlpha();
	void SetMapBump(XString mapBump);			// bump map (which by default uses luminance channel of the image)
	XString GetMapBump();
	void SetMapDisp(XString mapDisp);			// displacement map
	XString GetMapDisp();
	void SetMapDecal(XString mapDecal);			// stencil decal texture (defaults to 'matte' channel of the image)
	XString GetMapDecal();

	void Clear();

private:

	XString ms_MatName;			// unique material name

	glm::vec3 m_Ambient;		// ambient reflectivity, RGB
	glm::vec3 m_Diffuse;		// diffuse reflectivity, RGB
	glm::vec3 m_Specular;       // specular reflectivity, RGB
	float m_Shininess;			// specular shininess factor ( <= 0 for no specular component)
	float m_Tr;					// transparency
	glm::vec3 m_Tf;				// transmission filter, RGB
	UInt16 m_illum;				// illumination model
		//0. Color on and Ambient off
		//1. Color on and Ambient on
		//2. Highlight on
		//3. Reflection on and Ray trace on
		//4. Transparency: Glass on, Reflection : Ray trace on
		//5. Reflection : Fresnel on and Ray trace on
		//6. Transparency : Refraction on, Reflection : Fresnel off and Ray trace on
		//7. Transparency : Refraction on, Reflection : Fresnel on and Ray trace on
		//8. Reflection on and Ray trace off
		//9. Transparency : Glass on, Reflection : Ray trace off
		//10. Casts shadows onto invisible surfaces

	// texture maps
	XString m_map_Ka;			// ambient texture map
	XString m_map_Kd;			// diffuse texture map (normally same as the ambient texture map)
	XString m_map_Ks;			// specular color texture map
	XString m_map_Ns;			// specular highlight component
	XString m_map_d;			// alpha texture map
	XString m_map_bump;			// bump map (which by default uses luminance channel of the image)
	XString m_map_disp;			// displacement map
	XString m_map_decal;		// stencil decal texture (defaults to 'matte' channel of the image)
};

#endif // #ifndef MATERIAL_H
