// ShaderLib.cpp
//
// Putting all shaders in single source file to
//		1. avoid distribution headaches for shader files and 
//		2. solve age-old problem of where to keep them
//		3. am also finding this is just easier to work with than a bunch of little source files
//

#include "ShaderLib.h"

#define GLSL_VERSION "460"

// terrain shader =================================================================================

char m_VertexShaderTerrain[] =
{
	"#version " GLSL_VERSION "\n"

	// input per-vertex attributes

	"layout (location = 0) in vec3 VertexPosition;"		// P
	"layout (location = 1) in vec3 VertexNormal;"		// N
	"layout (location = 2) in vec2 VertexTexCoord;"		// T
	"layout (location = 3) in uint Flags;"				// 32 bit flags

	// output per-vertex attributes

	"out vec3 Position;"
	"out vec3 Normal;"
	"out vec2 TexCoord;"

	// uniform (per-frame) variables

	"uniform mat4 ModelViewMatrix;"
	"uniform mat3 NormalMatrix;"
	"uniform mat4 ProjectionMatrix;"
	"uniform mat4 MVP;"

	// horizontal clipping plane
	"uniform vec4 ClipPlane;"
	//"const vec4 plane = vec4(0, 0, 1, -1982.0);"	// culls below
	//"const vec4 plane = vec4(0, 0, -1, 1882.0);"	// culls above

	"void main()"
	"{"
		"TexCoord = VertexTexCoord;"
		"Normal = normalize(NormalMatrix * VertexNormal);"

		// Note: there are 2 output positions, one for our frag shader
		//		 and one for opengl
		"Position = vec3(ModelViewMatrix * vec4(VertexPosition, 1.0));"

		// gl_Position is vec4
		"gl_Position = MVP * vec4(VertexPosition, 1.0);"

		// signed distance from plane to vertex
		"gl_ClipDistance[0] = dot(vec4(VertexPosition, 1.0), ClipPlane);"
	"}"
};

char m_FragmentShaderTerrain[] =
{
	"#version " GLSL_VERSION "\n"

	// input per pixel parameters

	"in vec3 Position;"
	"in vec3 Normal;"
	"in vec2 TexCoord;"

	// output per pixel attributes

	"layout (location = 0) out vec4 FragColor;"

	// uniform (per-frame) variables

	"uniform bool EnableTextures = false;"
	"uniform sampler2D Tex1;"

	"struct LightInfo"
	"{"
		"vec3 Position;" 		// Light position in eye coords
		"vec3 Color;"	 		// R,G,B intensity
	"};"
	"uniform LightInfo Light;"

	"struct MaterialInfo"
	"{"
		"vec3 Ka;"            // Ambient reflectivity
		"vec3 Kd;"            // Diffuse reflectivity
		"vec3 Ks;"            // Specular reflectivity
		"float Shininess;"    // Specular shininess factor
	"};"
	"uniform MaterialInfo Material;"

	"void phongModel(vec3 pos, vec3 norm, out vec3 ambAndDiff, out vec3 spec)"
	"{"
		"vec3 s = normalize(Light.Position - pos);"
		"vec3 v = normalize(-pos.xyz);"
		"vec3 r = reflect(-s, norm);"
		"vec3 ambient = Light.Color * Material.Ka;"
		"float sDotN = max(dot(s, norm), 0.0);"
		"vec3 diffuse = Light.Color * Material.Kd * sDotN;"
		"spec = vec3(0.0);"
		"if (sDotN > 0.0)"
			"spec = Light.Color * Material.Ks * pow(max(dot(r, v), 0.0), Material.Shininess);"
		"ambAndDiff = ambient + diffuse;"
	"}"

	"void main()"
	"{"
		"vec3 ambAndDiff, spec;"
		"vec4 texColor = texture(Tex1, TexCoord);"

		"phongModel(Position, Normal, ambAndDiff, spec);"

		// to test, just return light gray
		//FragColor = vec4( 0.8, 0.8, 0.8, 1.0 );

		// another test, just sample texture directly with no lighting
		//FragColor = texColor;

		// full lighting + texture calculation
		"if (EnableTextures)\n"
			"FragColor = (vec4(ambAndDiff, 1.0) * texColor) + vec4(spec, 1.0);\n"
		"else\n"
			"FragColor = vec4(ambAndDiff, 1.0) + vec4(spec, 1.0);\n"
	"}"
};

// default position/normal/texture (PNT) shader ===================================================

char m_VertexShaderPNT[] = 
{
	"#version " GLSL_VERSION "\n"

	// input per-vertex attributes

	// must use 'layout' to avoid compiler optimizing
	// vars into another order
	"layout (location = 0) in vec3 VertexPosition;"		// P
	"layout (location = 1) in vec3 VertexNormal;"		// N
	"layout (location = 2) in vec2 VertexTexCoord;"		// T

	// output per-vertex attributes

	"out vec3 Position;"
	"out vec3 Normal;"
	"out vec2 TexCoord;"

	// uniform (per-frame) variables

	"uniform mat4 ModelViewMatrix;"
	"uniform mat3 NormalMatrix;"
	"uniform mat4 ProjectionMatrix;"
	"uniform mat4 MVP;"

	"void main()"
	"{"
		"TexCoord = VertexTexCoord;"
		"Normal = normalize( NormalMatrix * VertexNormal);"
		"Position = vec3( ModelViewMatrix * vec4(VertexPosition,1.0) );"
	
		"gl_Position = MVP * vec4(VertexPosition, 1.0);"
	"}"
};

char m_FragmentShaderPNT[] = 
{
	"#version " GLSL_VERSION "\n"

	// input per pixel parameters

	"in vec3 Position;"
	"in vec3 Normal;"
	"in vec2 TexCoord;"

	// output per pixel attributes

	"layout (location = 0) out vec4 FragColor;"

	// uniform (per-frame) variables

	"uniform bool EnableTextures = false;"
	"uniform sampler2D Tex1;"

	"struct LightInfo"
	"{"
		"vec3 Position;"  		// Light position in eye coords
		"vec3 Color;"	 		// R,G,B intensity
	"};"
	"uniform LightInfo Light;"

	"struct MaterialInfo"
	"{"
  		"vec3 Ka;"            // Ambient reflectivity
  		"vec3 Kd;"            // Diffuse reflectivity
  		"vec3 Ks;"            // Specular reflectivity
  		"float Shininess;"    // Specular shininess factor
	"};"
	"uniform MaterialInfo Material;"

	"void phongModel( vec3 pos, vec3 norm, out vec3 ambAndDiff, out vec3 spec ) "
	"{"
    	"vec3 s = normalize( Light.Position - pos );"
    	"vec3 v = normalize( -pos.xyz );"
    	"vec3 r = reflect( -s, norm );"
    	"vec3 ambient = Light.Color * Material.Ka;"
    	"float sDotN = max( dot(s,norm), 0.0 );"
    	"vec3 diffuse = Light.Color * Material.Kd * sDotN;"
    	"spec = vec3(0.0);"
    	"if ( sDotN > 0.0 )"
        	"spec = Light.Color * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess );"
    	"ambAndDiff = ambient + diffuse;"
	"}"

	"void main()"
	"{"
		"vec3 ambAndDiff, spec;"
		"vec4 texColor = texture( Tex1, TexCoord );"

		"phongModel( Position, Normal, ambAndDiff, spec );"

		// to test, just return light gray
		//"FragColor = vec4( 0.8, 0.8, 0.8, 1.0 );"

		// another test, just sample texture directly with no lighting
		//"FragColor = texColor;"

		// full lighting + texture calculation
		"if (EnableTextures)\n"
			"FragColor = (vec4( ambAndDiff, 1.0 ) * texColor) + vec4( spec, 1.0 );\n"
		"else\n"
			"FragColor = vec4( ambAndDiff, 1.0 ) + vec4( spec, 1.0 );\n"
	"}"
};

// default position/normal/color (PNC) shader ===================================================

char m_VertexShaderPNC[] =
{
	"#version " GLSL_VERSION "\n"

	// input per-vertex attributes

	"layout (location = 0) in vec3 VertexPosition;"		// P
	"layout (location = 1) in vec3 VertexNormal;"		// N
	"layout (location = 2) in vec3 VertexColor;"		// C

	// output per-vertex attributes

	"out vec3 Position;"
	"out vec3 Normal;"
	"out vec3 Color;"

	// uniform (per-frame) variables

	"uniform mat4 ModelViewMatrix;"
	"uniform mat3 NormalMatrix;"
	"uniform mat4 ProjectionMatrix;"
	"uniform mat4 MVP;"

	"void main()"
	"{"
		"Position = vec3( ModelViewMatrix * vec4(VertexPosition, 1.0) );"
		"Normal = normalize( NormalMatrix * VertexNormal );"
		"Color = VertexColor;"

		"gl_Position = MVP * vec4(VertexPosition, 1.0);"
	"}"
};

char m_FragmentShaderPNC[] =
{
	"#version " GLSL_VERSION "\n"

	// input per pixel parameters

	"in vec3 Position;"
	"in vec3 Normal;"
	"in vec3 Color;"

	// output per pixel attributes

	"layout (location = 0) out vec3 FragColor;"

	// uniform (per-frame) variables

	"struct LightInfo"
	"{"
		"vec3 Position;"  		// Light position in eye coords
		"vec3 Color;"			// R,G,B intensity
	"};"
	"uniform LightInfo Light;"

	"struct MaterialInfo"
	"{"
		"vec3 Ka;"            // Ambient reflectivity
		"vec3 Kd;"            // Diffuse reflectivity
		"vec3 Ks;"            // Specular reflectivity
		"float Shininess;"    // Specular shininess factor
	"};"
	"uniform MaterialInfo Material;"

	"void phongModel( vec3 pos, vec3 norm, out vec3 ambAndDiff, out vec3 spec ) "
	"{"
		"vec3 s = normalize( Light.Position - pos );"
		"vec3 v = normalize( -pos.xyz );"
		"vec3 r = reflect( -s, norm );"
		"vec3 ambient = Light.Color * Material.Ka;"
		"float sDotN = max( dot(s,norm), 0.0 );"
		"vec3 diffuse = Light.Color * Material.Kd * sDotN;"
		"spec = vec3(0.0);"
		"if ( sDotN > 0.0 )"
		"    spec = Light.Color * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess );"
		"ambAndDiff = ambient + diffuse;"
	"}"

	"void main()"
	"{"
		"vec3 ambAndDiff, spec;"

		"phongModel( Position, Normal, ambAndDiff, spec );"

		// to test, just return light gray
		//"FragColor = vec4( 0.8, 0.8, 0.8, 1.0 );"
	
		// another test, just use color with no lighting
		//"FragColor = Color;"
	
		// full lighting calculation
		"FragColor = (ambAndDiff * Color + spec);\n"
	"}"
};

// default position/color (PC) shader =============================================================

char m_VertexShaderPC[] =
{
	"#version " GLSL_VERSION "\n"

	// input per-vertex attributes

	"layout (location = 0) in vec3 VertexPosition;"			// P
	"layout (location = 1) in vec3 VertexColor;"			// C

	// output per-vertex attributes

	"out vec3 Color;"

	// uniform (per-frame) variables

	"uniform mat4 MVP;"

	"void main()"
	"{"
		"Color = VertexColor;"
		"gl_Position = MVP * vec4(VertexPosition, 1.0);"
	"}"
};

char m_FragmentShaderPC[] =
{
	"#version " GLSL_VERSION "\n"

	// input per-fragment attributes

	"in vec3 Color;"

	// output per-fragment attributes

	"layout (location = 0) out vec3 FragColor;"

	"void main()"
	"{"
		// just pass it thru
		"FragColor = Color;"
	"}"
};

// default position/color (PCT) shader =============================================================

char m_VertexShaderPCT[] =
{
	"#version " GLSL_VERSION "\n"

	// input per-vertex attributes

	"layout (location = 0) in vec3 VertexPosition;"		// P
	"layout (location = 1) in vec3 VertexColor;"		// C
	"layout (location = 2) in vec2 VertexTexCoord;"		// T

	// there are different ways to pass data between stages. 
	// here, i am using names, so the output "Color" here must
	// match the input "Color" in the fragment shader

	// output per-vertex attributes

	"out vec3 Color;"
	"out vec2 TexCoord;"

	// uniform (per-frame) variables

	"uniform mat4 MVP;"

	"void main()"
	"{"
		"Color = VertexColor;"
		"TexCoord = VertexTexCoord;"
		"gl_Position = MVP * vec4(VertexPosition, 1.0);"
	"}"
};

char m_FragmentShaderPCT[] =
{
	"#version " GLSL_VERSION "\n"

	// input per-fragment attributes

	"in vec3 Color;"		// these names must match out names in vertex shader
	"in vec2 TexCoord;"

	// output per-fragment attributes

	"layout (location = 0) out vec3 FragColor;"

	// uniform (per-frame) variables

	"uniform sampler2D Tex1;"

	"void main()"
	"{"
		"vec4 texColor = texture( Tex1, TexCoord );"
		"FragColor = Color * vec3( texColor );"
	"}"
};

// default screen space (SS) shader ===============================================================

char m_VertexShaderSS[] = 
{
	"#version " GLSL_VERSION "\n"

	// appears you can only use "layout" in vertex inputs and
	// shader outputs - so i guess there's no need to worry about
	// confusing/sharing slots

	// input per-vertex attributes

	"layout (location = 0) in vec2 VertexPosition;"
	"layout (location = 1) in vec3 VertexColor;"

	// output per-vertex attributes

	"out vec3 Color;"

	// uniform (per-frame) variables

	"uniform mat4 ProjectionMatrix;"

	"void main()"
	"{"
		"Color = VertexColor;"

		"gl_Position = vec4( VertexPosition, 0.0, 1.0 );"
		"gl_Position = ProjectionMatrix * gl_Position;"
	"}"
};

char m_FragmentShaderSS[] = 
{
	"#version " GLSL_VERSION "\n"

	// input per-vertex attributes

	"in vec3 Color;"

	// output per-vertex attributes

	"layout (location = 0) out vec3 FragColor;"

	"void main() "
	"{"
		// we want this fragment rendered on top of everything else
		"gl_FragDepth = 0.0;"

		"FragColor = Color;"
	"}"
};

// default screen space w/color + texture (SSCT) shader ==========================================

char m_VertexShaderSSCT[] =
{
	"#version " GLSL_VERSION "\n"

	// appears you can only use "layout" in vertex inputs and
	// shader outputs - so i guess there's no need to worry about
	// confusing/sharing slots

	// input per-vertex attributes

	"layout (location = 0) in vec2 VertexPosition;"		// P (screen space)
	"layout (location = 1) in vec3 VertexColor;"		// C
	"layout (location = 2) in vec2 VertexTexCoord;"		// T

	// output per-vertex attributes

	"out vec3 Color;"
	"out vec2 TexCoord;"

	// uniform (per-frame) variables

	"uniform mat4 ProjectionMatrix;"

	"void main()"
	"{"
		"Color = VertexColor;"
		"TexCoord = VertexTexCoord;"

		"gl_Position = vec4( VertexPosition, 0.0, 1.0 );"
		"gl_Position = ProjectionMatrix * gl_Position;"
	"}"
};

char m_FragmentShaderSSCT[] =
{
	"#version " GLSL_VERSION "\n"

	// input per-vertex attributes

	"in vec3 Color;"
	"in vec2 TexCoord;"

	// output per-pixel attributes

	"layout (location = 0) out vec3 FragColor;"

	// uniform (per-frame) variables

	"uniform sampler2D Tex1;"

	"void main() "
	"{"
		// we want this fragment rendered on top of everything else
		"gl_FragDepth = 0.0;"

		"vec4 texColor = texture( Tex1, TexCoord );"
		"FragColor = Color * vec3( texColor );"
		//"FragColor = vec3( texColor );"
	"}"
};

// image shader ==================================================================================
// like screen space, but has x/y offsets for easy image placement
//

char m_VertexShaderImage[] =
{
	"#version " GLSL_VERSION "\n"

	// input per-vertex attributes

	"layout (location = 0) in vec2 VertexPosition;"
	"layout (location = 1) in vec3 VertexColor;"

	// output per-vertex attributes

	"out vec3 Color;"

	// uniform (per-frame) variables

	"uniform mat4 ProjectionMatrix;"
	"uniform float xoff;"					// current xy offsets
	"uniform float yoff;"

	"void main()"
	"{"
		"Color = VertexColor;"

		"gl_Position = vec4(VertexPosition.x + xoff, VertexPosition.y + yoff, 0.0, 1.0);"
		"gl_Position = ProjectionMatrix * gl_Position;"
	"}"
};

char m_FragmentShaderImage[] =
{
	"#version " GLSL_VERSION "\n"

	// input per-vertex attributes

	"in vec3 Color;"

	// output per-pixel attributes

	"layout (location = 0) out vec3 FragColor;"

	"void main()"
	"{"
		// we want this fragment rendered on top of everything else
		"gl_FragDepth = 0.0;"

		"FragColor = Color;"
	"}"
};

// skybox shader ==================================================================================

char m_VertexShaderSkyBox[] =
{
	"#version " GLSL_VERSION "\n"

	"layout (location = 0) in vec3 aPos;"

	"out vec3 TexCoords;"

	"uniform mat4 projection;"
	"uniform mat4 view;"

	"void main()"
	"{"
		"TexCoords = aPos;"
		"gl_Position = projection * view * vec4(aPos, 1.0);"
	"}"
};

char m_FragmentShaderSkyBox[] =
{
	"#version " GLSL_VERSION "\n"

	"in vec3 TexCoords;"

	"layout (location = 0) out vec4 FragColor;"

	"uniform samplerCube skybox;"

	"void main()"
	"{"
		"FragColor = texture(skybox, TexCoords);"
	"}"
};

//=================================================================================================

const char* ShaderLib::GetVertexShaderPC()
{
	return m_VertexShaderPC;
}

const char* ShaderLib::GetFragmentShaderPC()
{
	return m_FragmentShaderPC;
}

const char* ShaderLib::GetVertexShaderPCT()
{
	return m_VertexShaderPCT;
}

const char* ShaderLib::GetFragmentShaderPCT()
{
	return m_FragmentShaderPCT;
}

const char* ShaderLib::GetVertexShaderPNT()
{
	return m_VertexShaderPNT;
}

const char* ShaderLib::GetFragmentShaderPNT()
{
	return m_FragmentShaderPNT;
}

const char* ShaderLib::GetVertexShaderPNC()
{
	return m_VertexShaderPNC;
}

const char* ShaderLib::GetFragmentShaderPNC()
{
	return m_FragmentShaderPNC;
}

const char* ShaderLib::GetVertexShaderSS()
{
	return m_VertexShaderSS;
}

const char* ShaderLib::GetFragmentShaderSS()
{
	return m_FragmentShaderSS;
}

const char* ShaderLib::GetVertexShaderSSCT()
{
	return m_VertexShaderSSCT;
}

const char* ShaderLib::GetFragmentShaderSSCT()
{
	return m_FragmentShaderSSCT;
}

const char* ShaderLib::GetVertexShaderImage()
{
	return m_VertexShaderImage;
}

const char* ShaderLib::GetFragmentShaderImage()
{
	return m_FragmentShaderImage;
}

const char* ShaderLib::GetVertexShaderSkyBox()
{
	return m_VertexShaderSkyBox;
}

const char* ShaderLib::GetFragmentShaderSkyBox()
{
	return m_FragmentShaderSkyBox;
}

const char* ShaderLib::GetVertexShaderTerrain()
{
	return m_VertexShaderTerrain;
}

const char* ShaderLib::GetFragmentShaderTerrain()
{
	return m_FragmentShaderTerrain;
}
