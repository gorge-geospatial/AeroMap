// Model_OBJ.cpp
// Manager for WaveFront OBJ files
//
// File format:
// 
//Vertex data:
//
//    v 	Geometric vertices
//    vt 	Texture vertices
//    vn 	Vertex normals
//    vp 	Parameter space vertices
//
//Free-form curve/surface attributes:
//
//    deg 	Degree
//    bmat 	Basis matrix
//    step 	Step size
//    cstype 	Curve or surface type
//
//Elements:
//
//    p 		Point
//    l 		Line
//    f 		Face
//    curv 		Curve
//    curv2 	2D curve
//    surf 		Surface
//
//Free-form curve/surface body statements:
//
//    parm 	Parameter values
//    trim 	Outer trimming loop
//    hole 	Inner trimming loop
//    scrv 	Special curve
//    sp 	Special point
//    end 	End statement
//
//Connectivity between free-form surfaces:
//
//    con 	Connect
//
//Grouping:
//
//    g 	Group name
//    s 	Smoothing group
//    mg 	Merging group
//    o 	Object name
//
//Display/render attributes:
//
//    bevel 		Bevel interpolation
//    c_interp 		Color interpolation
//    d_interp 		Dissolve interpolation
//    lod 			Level of detail
//    usemtl 		Material name
//    mtllib 		Material library
//    shadow_obj 	Shadow casting
//    trace_obj 	Ray tracing
//    ctech 		Curve approximation technique
//    stech 		Surface approximation technique
//
//	examples:
//
//		# List of geometric vertices, with(x, y, z[, w]) coordinates, w is optional and defaults to 1.0.
//		v 0.123 0.234 0.345 1.0
//		v ...
//		...
//		# List of texture coordinates, in(u, v[, w]) coordinates, these will vary between 0 and 1, w is optional and defaults to 0.
//		vt 0.500 1[0]
//		vt ...
//		...
//		# List of vertex normals in(x, y, z) form; normals might not be unit vectors.
//		vn 0.707 0.000 0.707
//		vn ...
//		...
//		# Parameter space vertices in(u[, v][, w]) form; free form geometry statement(see below)
//		vp 0.310000 3.210000 2.100000
//		vp ...
//		...
//		# Polygonal face element(see below)
//		f 1 2 3
//		f 3/1 4/2 5/3
//		f 6/4/1 3/5/3 7/6/5
//		f ...
//		...
//		# Lines beginning with a hash character (#) are comments.

#include <stdio.h>

#include "TextFile.h"
#include "Logger.h"
#include "Model_OBJ.h"

Model_OBJ::Model_OBJ()
	: Model()
{
}

Model_OBJ::~Model_OBJ()
{
}

bool Model_OBJ::Load(const char* fileName)
{
	// Load WaveFront OBJ format file.
	//

	FILE* pFile = fopen(fileName, "rt");
	if (pFile == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open file '%s'.", fileName);
		return false;
	}
	fclose(pFile);

	ms_FileName = fileName;

//TODO:
//gradually replacing these with base types

	// vertex entry:
	// v  0.855968 0.627276 -0.278121
	// (some applications support vertex colors, by putting red, green and blue values after x y and z. The color values range from 0 to 1)

	struct _VertexType
	{
		float x;
		float y;
		float z;
		float w;		// optional, defaults to 1.0

		_VertexType()
		{
			Clear();
		}
		void Clear()
		{
			x = 0.0;
			y = 0.0;
			z = 0.0;
			w = 1.0;		// optional, defaults to 1.0
		}
	};

	// normal entry:
	// vn  0.499315 0.000000 0.026361

	struct _NormalType
	{
		float x;
		float y;
		float z;

		_NormalType()
		{
			Clear();
		}
		void Normalize()
		{
			// since obj file normals may not be unit vectors,
			// this method provided to make them so
			float mag = sqrtf(x*x + y*y + z*z);
			float sf = 1.0F / mag;
			x *= sf;
			y *= sf;
			z *= sf;
		}
		void Clear()
		{
			x = 0.0;
			y = 0.0;
			z = 0.0;
		}
	};

	// face entry:
	// f 77/60/77 78/61/78 117/82/117 - vx/tx/n vx/tx/n vx/tx/n

	struct _FaceType
	{
		UInt32 Vx0;		// triangle vertex indices
		UInt32 Vx1;
		UInt32 Vx2;

		UInt32 Tx0;		// texture coordinate indices
		UInt32 Tx1;
		UInt32 Tx2;

		UInt32 N0;		// normal indices
		UInt32 N1;
		UInt32 N2;

		_FaceType()		// constructor
		{
			Clear();
		}
		void Clear()
		{
			Vx0 = Vx1 = Vx2 = 0;
			Tx0 = Tx1 = Tx2 = 0;
			N0 = N1 = N2 = 0;
		}
	};

	std::vector<TexCoord> texList;
	std::vector<_FaceType> faceList;
	std::vector<_VertexType> vxList;
	std::vector<VEC3> normalList;
	std::vector<XString> groupList;				// named face groups

	_VertexType vx;
	_FaceType face;
	VEC3 N;
	TexCoord tex;

	TextFile textFile(fileName);

	XString line;
	for (int i = 0; i < textFile.GetLineCount(); ++i)
	{
		line = textFile.GetLine(i).c_str();

		// remove comments
		line.TruncateAt('#');

		int tokenCount = line.Tokenize(" ");
		if (tokenCount > 0)
		{
			if (line.GetToken(0).Compare("v"))
			{
				// v  0.900018 0.627276 0.000000
				vx.Clear();
				if (tokenCount > 3)
				{
					vx.x = static_cast<float>(line.GetToken(1).GetDouble());
					vx.y = static_cast<float>(line.GetToken(2).GetDouble());
					vx.z = static_cast<float>(line.GetToken(3).GetDouble());
					vxList.push_back(vx);
				}
				else
				{
					Logger::Write(__FUNCTION__, "Invalid 'v' entry: %s", line.c_str());
				}
			}
			else if (line.GetToken(0).Compare("vt"))
			{
				// texture coordinate entry:
				// vt  0.792723 0.009391 0.000000
				tex.Clear();
				if (tokenCount > 2)
				{
					tex.u = static_cast<float>(line.GetToken(1).GetDouble());
					tex.v = static_cast<float>(line.GetToken(2).GetDouble());
					if (tokenCount > 3)
						tex.w = static_cast<float>(line.GetToken(3).GetDouble());
					texList.push_back(tex);
				}
				else
				{
					Logger::Write(__FUNCTION__, "Invalid 'vt' entry: %s", line.c_str());
				}
			}
			else if (line.GetToken(0).Compare("vn"))
			{
				// vn  -0.000000 -0.500010 -0.000090
				//N.Clear();
				if (tokenCount > 3)
				{
					N.x = static_cast<float>(line.GetToken(1).GetDouble());
					N.y = static_cast<float>(line.GetToken(2).GetDouble());
					N.z = static_cast<float>(line.GetToken(3).GetDouble());
					N = N.Unit();
					normalList.push_back(N);
				}
				else
				{
					Logger::Write(__FUNCTION__, "Invalid 'v' entry: %s", line.c_str());
				}
			}
			else if (line.GetToken(0).Compare("f"))
			{
				  // # Polygonal face element
  				  // f 1 2 3							- vertices only
  				  // f 3/1 4/2 5/3						- vertex and texture
  				  // f 6/4/1 3/5/3 7/6/5				- vertex, texture and normal

				face.Clear();
				if (tokenCount > 3)
				{
					// Docs say faces have 3 or more vx, not 3, so
					// need to triangulate any faces with more than
					// 3 vx

					// each additional vx past 'f' and vx 1,2,3 represents
					// another triangle
					int triCount = tokenCount - 3;
					XString f1 = line.GetToken(1);
					for (int tri = 0; tri < triCount; ++tri)
					{
						// this assumes faces can be trianglulated as
						// (1,2,3), (1,3,4), (1,4,5), etc.
						XString f2 = line.GetToken(2 + tri);
						XString f3 = line.GetToken(3 + tri);

						int count1 = f1.Tokenize("/");
						int count2 = f2.Tokenize("/");
						int count3 = f3.Tokenize("/");

						if (count1 == count2 && count2 == count3)
						{
							face.Vx0 = f1.GetToken(0).GetInt();
							face.Vx1 = f2.GetToken(0).GetInt();
							face.Vx2 = f3.GetToken(0).GetInt();

							switch (count1) {
							case 1:		// vertex only

								break;

							case 2:		// vertex and texture/normal

								// could be "1/4" for vx/tex or "1//4" for vx/normal
								if (f1.Find("//") == -1)
									face.Tx0 = f1.GetToken(1).GetInt();
								else
									face.N0 = f1.GetToken(1).GetInt();
								if (f2.Find("//") == -1)
									face.Tx1 = f2.GetToken(1).GetInt();
								else
									face.N1 = f2.GetToken(1).GetInt();
								if (f3.Find("//") == -1)
									face.Tx2 = f3.GetToken(1).GetInt();
								else
									face.N2 = f3.GetToken(1).GetInt();
								break;

							case 3:		// vertex, texture and normal

								face.Tx0 = f1.GetToken(1).GetInt();
								face.Tx1 = f2.GetToken(1).GetInt();
								face.Tx2 = f3.GetToken(1).GetInt();

								face.N0 = f1.GetToken(2).GetInt();
								face.N1 = f2.GetToken(2).GetInt();
								face.N2 = f3.GetToken(2).GetInt();
								break;

							default:

								Logger::Write(__FUNCTION__, "Invalid 'f' entry: %s", line.c_str());
								break;
							}
							faceList.push_back(face);
						}
						else
						{
							Logger::Write(__FUNCTION__, "Invalid 'f' entry: %s", line.c_str());
						}
					}
				}
				else
				{
					Logger::Write(__FUNCTION__, "Invalid 'f' entry: %s", line.c_str());
				}
			}
			else if (line.GetToken(0).Compare("s"))
			{
				//TODO:
				//smoothing groups not implemented
				//
				// Smooth shading across polygons is enabled by smoothing groups.
				//
				// s 1
				// ...
				// # Smooth shading can be disabled as well.
				//  s off
				//  ...
				//Logger::Write(__FUNCTION__, "Unrecognized entry: %s", line.c_str());
			}
			else if (line.GetToken(0).Compare("g"))
			{
				// Named objects and polygon groups are specified via the following tags.
				//
				// o [object name]
				// ...
				//  g [group name]
				//  ...
				if (line.GetTokenCount() == 2)
				{
					groupList.push_back(line.GetToken(1));
				}
				else
				{
					Logger::Write(__FUNCTION__, "Invalid 'g' entry: %s", line.c_str());
				}
			}
			else if (line.GetToken(0).Compare("o"))
			{
				Logger::Write(__FUNCTION__, "Unrecognized entry: %s", line.c_str());
			}
			else if (line.GetToken(0).Compare("mtllib"))
			{
				// specify material file

				if (line.GetTokenCount() > 1)
				{
					// material file name could have spaces, so take everything from
					// end of "mtllib" to end
					XString matFileName = line.Mid(7);
					LoadMaterials(matFileName.c_str());
				}
				else
				{
					Logger::Write(__FUNCTION__, "Invalid 'mtllib' entry: %s", line.c_str());
				}
			}
			else if (line.GetToken(0).Compare("usemtl"))
			{
				
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unrecognized entry: %s", line.c_str());
			}
		}
	}

	// validate data structures

	for (unsigned int i = 0; i < faceList.size(); ++i)
	{
		// validate vertex indices
		if (faceList[i].Vx0 > vxList.size())
			Logger::Write(__FUNCTION__, "Invalid vertex #0 index (%lu) at face[%lu]", faceList[i].Vx0, i);
		if (faceList[i].Vx1 > vxList.size())
			Logger::Write(__FUNCTION__, "Invalid vertex #1 index (%lu) at face[%lu]", faceList[i].Vx1, i);
		if (faceList[i].Vx2 > vxList.size())
			Logger::Write(__FUNCTION__, "Invalid vertex #2 index (%lu) at face[%lu]", faceList[i].Vx2, i);

		if (texList.size() > 0)
		{
			if (faceList[i].Tx0 > texList.size())
				Logger::Write(__FUNCTION__, "Invalid texture #0 index (%lu) at face[%lu]", faceList[i].Tx0, i);
			if (faceList[i].Tx1 > texList.size())
				Logger::Write(__FUNCTION__, "Invalid texture #1 index (%lu) at face[%lu]", faceList[i].Tx1, i);
			if (faceList[i].Tx2 > texList.size())
				Logger::Write(__FUNCTION__, "Invalid texture #2 index (%lu) at face[%lu]", faceList[i].Tx2, i);
		}

		if (normalList.size() > 0)
		{
			if (faceList[i].N0 > normalList.size())
				Logger::Write(__FUNCTION__, "Invalid normal #0 index (%lu) at face[%lu]", faceList[i].N0, i);
			if (faceList[i].N1 > normalList.size())
				Logger::Write(__FUNCTION__, "Invalid normal #1 index (%lu) at face[%lu]", faceList[i].N1, i);
			if (faceList[i].N2 > normalList.size())
				Logger::Write(__FUNCTION__, "Invalid normal #2 index (%lu) at face[%lu]", faceList[i].N2, i);
		}
	}

	// now set up base class structures

	MeshType mesh;		// only support single mesh (but may need to expand)
	for (int i = 0; i < vxList.size(); ++i)
	{
		VertexType vx;
		vx.P = VEC3(vxList[i].x, vxList[i].y, vxList[i].z);
		mesh.vVx.push_back(vx);
	}
	if (texList.size() > 0)
	{
		for (int i = 0; i < texList.size(); ++i)
		{
			TexCoord tx;
			tx.u = texList[i].u;
			tx.v = texList[i].v;
			tx.w = texList[i].w;
			mesh.vTex.push_back(tx);
		}
	}
	for (int i = 0; i < faceList.size(); ++i)
	{
		FaceType face;
		face.vx0 = faceList[i].Vx0 - 1;		// "-1" because file was 1-based
		face.vx1 = faceList[i].Vx1 - 1;
		face.vx2 = faceList[i].Vx2 - 1;
		if (texList.size() > 0)
		{
			face.tx0 = faceList[i].Tx0 - 1;
			face.tx1 = faceList[i].Tx1 - 1;
			face.tx2 = faceList[i].Tx2 - 1;
		}
		mesh.vFace.push_back(face);

		if (normalList.size() > 0)
		{
			// can't load the normals directly from normalList[], they do not correspond
			// to vxList[] by index; need to get indices for normal & associated vx from
			// face structure
			int v = faceList[i].N0 - 1;
			mesh.vVx[face.vx0].N = VEC3(normalList[v].x, normalList[v].y, normalList[v].z);
			v = faceList[i].N1 - 1;
			mesh.vVx[face.vx1].N = VEC3(normalList[v].x, normalList[v].y, normalList[v].z);
			v = faceList[i].N2 - 1;
			mesh.vVx[face.vx2].N = VEC3(normalList[v].x, normalList[v].y, normalList[v].z);
		}
	}
	mv_Mesh.push_back(mesh);

	// because XDrive currently only supports a single texture
	// per model (which is infinitely more than it did for the
	// first 20 years of its life!), just grab the first one 
	// we come to.
	if (m_Texture.IsValid() == false)
	{
		for (Material mat : mv_Mat)
		{
			XString strTextureFile;
			if (mat.GetMapDiffuse().GetLength() > 0)
				strTextureFile = mat.GetMapDiffuse();
			else if (mat.GetMapAmbient().GetLength() > 0)
				strTextureFile = mat.GetMapAmbient();

			if (strTextureFile.GetLength() > 0)
			{
				strTextureFile = GetTextureFileName(strTextureFile.c_str());
				if (m_Texture.Load(strTextureFile.c_str()) == false)
					Logger::Write(__FUNCTION__, "Unable to load texture file: %s", strTextureFile.c_str());
			}
		}
	}

	return true;
}

void Model_OBJ::LoadMaterials(const char* matFileName)
{
	// Load material(s) from .mtl file.
	//
	// material file format:
	//
	// newmtl wire_228214153			# material name
	//		Ka 0.8941 0.8392 0.6000		# ambient 
	//		Kd 0.8941 0.8392 0.6000		# diffuse
	//		Ks 0.3500 0.3500 0.3500		# specular
	//		Ns 32						# shininess
	//		Tr 0						# transparency (some implementations use 'd' instead of 'Tr')
	//		Tf 1 1 1					# transmission filter (r,g,b)
	//		illum 2						# illumination model
	//		map_Ka [filename]			# the ambient texture map
	//		map_Kd [filename]			# the diffuse texture map (normally same as the ambient texture map)
	//		map_Ks [filename]           # specular color texture map
	//		map_Ns [filename]			# specular highlight component
	//		map_d [filename]			# the alpha texture map
	//		map_bump [filename]			# some implementations use 'map_bump' instead of 'bump' below
	//		bump [filename]				# bump map (which by default uses luminance channel of the image)
	//		disp [filename]				# displacement map
	//		decal [filename]			# stencil decal texture (defaults to 'matte' channel of the image)

	XString pathName = ms_FileName.GetPathName();
	TextFile matFile(XString::CombinePath(pathName.c_str(), matFileName));
	if (matFile.GetLineCount() > 0)
	{
		Material mat;
		for (int i = 0; i < matFile.GetLineCount(); ++i)
		{
			XString str = matFile.GetLine(i).c_str();
			int tokenCount = str.Tokenize(" \t");
			if (tokenCount > 0)
			{
				if (str.GetToken(0).Compare("newmtl"))
				{
					mat.SetName(str.GetToken(1));
				}
				else if (str.GetToken(0).Compare("Ka"))
				{
					float r = static_cast<float>(str.GetToken(1).GetDouble());
					float g = static_cast<float>(str.GetToken(2).GetDouble());
					float b = static_cast<float>(str.GetToken(3).GetDouble());
					mat.SetAmbient(vec3(r, g, b));
				}
				else if (str.GetToken(0).Compare("Kd"))
				{
					float r = static_cast<float>(str.GetToken(1).GetDouble());
					float g = static_cast<float>(str.GetToken(2).GetDouble());
					float b = static_cast<float>(str.GetToken(3).GetDouble());
					mat.SetDiffuse(vec3(r, g, b));
				}
				else if (str.GetToken(0).Compare("Ks"))
				{
					float r = static_cast<float>(str.GetToken(1).GetDouble());
					float g = static_cast<float>(str.GetToken(2).GetDouble());
					float b = static_cast<float>(str.GetToken(3).GetDouble());
					mat.SetSpecular(vec3(r, g, b));
				}
				else if (str.GetToken(0).Compare("Ns"))
				{
					float Ns = static_cast<UInt16>(str.GetToken(1).GetInt());
					mat.SetShininess(Ns);
				}
				else if (str.GetToken(0).Compare("Tr") || str.GetToken(0).Compare("d"))
				{
					float Tr = static_cast<float>(str.GetToken(1).GetDouble());
					mat.SetTransparency(Tr);
				}
				else if (str.GetToken(0).Compare("Tf"))
				{
					float r = static_cast<UInt16>(str.GetToken(1).GetInt());
					float g = static_cast<UInt16>(str.GetToken(2).GetInt());
					float b = static_cast<UInt16>(str.GetToken(3).GetInt());
					mat.SetTransmission(vec3(r, g, b));
				}
				else if (str.GetToken(0).Compare("illum"))
				{
					UInt16 illum = static_cast<UInt16>(str.GetToken(1).GetInt());
					mat.SetIllum(illum);
				}
				else if (str.GetToken(0).Compare("map_Ka"))
				{
					mat.SetMapAmbient(str.GetToken(1));
				}
				else if (str.GetToken(0).Compare("map_Kd"))
				{
					mat.SetMapDiffuse(str.GetToken(1));
				}
				else if (str.GetToken(0).Compare("map_Ks"))
				{
					mat.SetMapSpecular(str.GetToken(1));
				}
				else if (str.GetToken(0).Compare("map_Ns"))
				{
					mat.SetMapNs(str.GetToken(1));
				}
				else if (str.GetToken(0).Compare("map_bump") || str.GetToken(0).Compare("bump"))
				{
					mat.SetMapBump(str.GetToken(1));
				}
				else if (str.GetToken(0).Compare("disp"))
				{
					mat.SetMapDisp(str.GetToken(1));
				}
				else if (str.GetToken(0).Compare("decal"))
				{
					mat.SetMapDecal(str.GetToken(1));
				}
				else
				{
					Logger::Write(__FUNCTION__, "Unrecognized entry in material file '%s': [%s]", matFileName, str.c_str());
				}
			}
		}
		mv_Mat.push_back(mat);
	}
}

bool Model_OBJ::Save(const char* fileName)
{
	// Write model in OBJ format.
	//

	FILE* pFile = fopen(fileName, "wt");
	if (pFile == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open file for writing: '%s'.", fileName);
		return false;
	}

	ms_FileName = fileName;
	
	fprintf(pFile, "# WaveFront OBJ file\n");
	fprintf(pFile, "# Written by XDrive\n");
	fprintf(pFile, "\n");

	// write material library entry
	if (GetMaterialCount() > 0)
	{
		XString matFileName = ms_FileName;
		if (matFileName.EndsWithNoCase(".obj"))
			matFileName.DeleteRight(4);

		matFileName += ".mtl";
		SaveMaterials(matFileName);

		fprintf(pFile, "mtllib %s\n", matFileName.GetFileName().c_str());
		fprintf(pFile, "\n");
	}

	for (int meshIdx = 0; meshIdx < GetMeshCount(); ++meshIdx)
	{
		MeshType* pMesh = &mv_Mesh[meshIdx];

		// write group name (interchangeable with mesh?)
		
		XString name = pMesh->Name;
		name.Trim();
		if (name.GetLength() == 0)
			name = name.Format("Mesh%d", meshIdx);
		fprintf(pFile, "g %s\n", name.c_str());
		fprintf(pFile, "\n");

		// write vertices
		//		# List of geometric vertices, with(x, y, z[, w]) coordinates, w is optional and defaults to 1.0.
		//		v 0.123 0.234 0.345 1.0
		//		v ...

		for (VertexType vx : pMesh->vVx)
		{
			fprintf(pFile, "v %0.4f %0.4f %0.4f\n", vx.P.x, vx.P.y, vx.P.z);
		}
		fprintf(pFile, "\n");

		// write texture coordinates, if applicable
		if (pMesh->vTex.size() > 0)
		{
			for (TexCoord tx : pMesh->vTex)
			{
				fprintf(pFile, "vt %0.4f %0.4f\n", tx.u, tx.v);
			}
			fprintf(pFile, "\n");
		}

		// write vertex normals
		//		# List of vertex normals in(x, y, z) form; normals might not be unit vectors.
		//		vn 0.707 0.000 0.707
		//		vn ...

		CalculateNormals();
		for (VertexType vx : pMesh->vVx)
		{
			fprintf(pFile, "vn %0.4f %0.4f %0.4f\n", vx.N.x, vx.N.y, vx.N.z);
		}
		fprintf(pFile, "\n");

		// write faces
		//		# Polygonal face element(see below)
		//		f 1 2 3
		//		f 3/1 4/2 5/3
		//		f 6/4/1 3/5/3 7/6/5
		//		f ...
		//		...

		for (FaceType face : pMesh->vFace)
		{
			int vx0 = face.vx0 + 1;
			int vx1 = face.vx1 + 1;
			int vx2 = face.vx2 + 1;
			int tx0 = face.tx0 + 1;
			int tx1 = face.tx1 + 1;
			int tx2 = face.tx2 + 1;

			// order is vx/tex/normal, but only vx is required
			// my models always have normals & they are always the same index as vx
			if (pMesh->vTex.size() > 0)
				fprintf(pFile, "f %d/%d/%d %d/%d/%d %d/%d/%d \n", vx0, tx0, vx0, vx1, tx1, vx1, vx2, tx2, vx2);
			else
				fprintf(pFile, "f %d//%d %d//%d %d//%d \n", vx0, vx0, vx1, vx1, vx2, vx2);
		}
		fprintf(pFile, "\n");
	}

	fclose(pFile);

	return true;
}

void Model_OBJ::SaveMaterials(const char* matFileName)
{
	// Save material(s) to .mtl file.
	//
	// material file format:
	//
	// newmtl wire_228214153			# material name
	//		Ka 0.8941 0.8392 0.6000		# ambient 
	//		Kd 0.8941 0.8392 0.6000		# diffuse
	//		Ks 0.3500 0.3500 0.3500		# specular
	//		Ns 32						# shininess
	//		Tr 0						# transparency (some implementations use 'd' instead of 'Tr')
	//		Tf 1 1 1					# transmission filter (r,g,b)
	//		illum 2						# illumination model
	//		map_Ka [filename]			# the ambient texture map
	//		map_Kd [filename]			# the diffuse texture map (normally same as the ambient texture map)
	//		map_Ks [filename]           # specular color texture map
	//		map_Ns [filename]			# specular highlight component
	//		map_d [filename]			# the alpha texture map
	//		map_bump [filename]			# some implementations use 'map_bump' instead of 'bump' below
	//		bump [filename]				# bump map (which by default uses luminance channel of the image)
	//		disp [filename]				# displacement map
	//		decal [filename]			# stencil decal texture (defaults to 'matte' channel of the image)

	if (GetMaterialCount() == 0)
		return;

	FILE* pFile = fopen(matFileName, "wt");
	if (pFile == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open file for writing: '%s'.", matFileName);
		return;
	}

	for (int i = 0; i < GetMaterialCount(); ++i)
	{
		Material* pMat = &mv_Mat[i];
		fprintf(pFile, "newmtl %s\n", pMat->GetName().c_str());
		fprintf(pFile, "Ka %0.4f %0.4f %0.4f\n", pMat->GetAmbient().r, pMat->GetAmbient().g, pMat->GetAmbient().b);
		fprintf(pFile, "Kd %0.4f %0.4f %0.4f\n", pMat->GetDiffuse().r, pMat->GetDiffuse().g, pMat->GetDiffuse().b);
		fprintf(pFile, "Ks %0.4f %0.4f %0.4f\n", pMat->GetSpecular().r, pMat->GetSpecular().g, pMat->GetSpecular().b);
		fprintf(pFile, "Ns %0.1f\n", pMat->GetShininess());
		fprintf(pFile, "Tr %0.1f\n", pMat->GetTransparency());
		fprintf(pFile, "Tf %0.4f %0.4f %0.4f\n", pMat->GetTransmission().r, pMat->GetTransmission().g, pMat->GetTransmission().b);
		fprintf(pFile, "illum %d\n", pMat->GetIllum());
		if (pMat->GetMapAmbient().GetLength() > 0)
			fprintf(pFile, "map_Ka %s\n", pMat->GetMapAmbient().c_str());
		if (pMat->GetMapDiffuse().GetLength() > 0)
			fprintf(pFile, "map_Kd %s\n", pMat->GetMapDiffuse().c_str());
		if (pMat->GetMapSpecular().GetLength() > 0)
			fprintf(pFile, "map_Ks %s\n", pMat->GetMapSpecular().c_str());
		if (pMat->GetMapNs().GetLength() > 0)
			fprintf(pFile, "map_Ns %s\n", pMat->GetMapNs().c_str());
		if (pMat->GetMapAlpha().GetLength() > 0)
			fprintf(pFile, "map_d %s\n", pMat->GetMapAlpha().c_str());
		if (pMat->GetMapBump().GetLength() > 0)
			fprintf(pFile, "map_bump %s\n", pMat->GetMapBump().c_str());
		if (pMat->GetMapDisp().GetLength() > 0)
			fprintf(pFile, "disp %s\n", pMat->GetMapDisp().c_str());
		if (pMat->GetMapDecal().GetLength() > 0)
			fprintf(pFile, "decal %s\n", pMat->GetMapDecal().c_str());
		fprintf(pFile, "\n");
	}

	fclose(pFile);
}
