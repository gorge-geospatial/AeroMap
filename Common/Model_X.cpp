// Model_X.cpp
// X file manager class.
// 
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//
// All data management handled in base Model class. To support
// a specific polygonal model format, subclass Model and override
// Load()/Save().
//

#include "Model_X.h"			// interface to this class

#define MAX_MESH_NAME 32
#define X_DELIMITERS ",;{} \n\r\t\""		// x file delimiters

Model_X::Model_X()
	: Model()
{
	mv_Mesh.clear();
	mv_Mesh.reserve(16);
}

Model_X::~Model_X()
{
}

bool Model_X::Load(const char* fileName)
{
	// Load a polygonal model from DirectX .X file.

	enum XFILE_SECTION
	{
		SECT_NONE,
		SECT_VERTEX,
		SECT_FACE,
		SECT_NORMAL_VX,
		SECT_NORMAL_FACE,
		SECT_TEXTURE_COORD,
		SECT_MATERIAL,
	};

	// current block, used for loading x files w/recursive elements

	struct BlockType
	{
		XString Name[255];		// block name
		char Type;				// 'F' = frame, 'M' = mesh, 'B' = untyped block (just {})
		bool HasTransform;		// true => .Transform property valid
		//MATRIX Transform;		// 4x4 frame transformation matrix
	};

	MeshType mesh;
	unsigned int normalCtr = 0;
	unsigned int normalCount = 0;
	unsigned int textureCtr = 0;
	unsigned int textureCount = 0;
	unsigned int vxCount = 0;
	unsigned int faceCount = 0;
	XFILE_SECTION curSection = SECT_NONE;
	char scur_tok[80] = { 0 };

	mv_Mesh.clear();

	FILE* pFile = fopen(fileName, "rt");
	if (pFile == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open file '%s'.", fileName);
		return false;
	}

	// set it early, it may get used during load
	ms_FileName = fileName;

	while (!feof(pFile))
	{
		GetXFileToken(pFile, scur_tok);
		if (strcmp(scur_tok, "mesh") == 0)	// encountered mesh definition
		{
			// if mesh pending
			if (vxCount > 0)
			{
				mv_Mesh.push_back(mesh);
			}

			// start a new one
			mesh.Clear();

			curSection = SECT_VERTEX;

			vxCount = 0;
			faceCount = 0;
			normalCount = 0;
			textureCount = 0;
			textureCtr = 0;

			// mesh name
			GetXFileToken(pFile, scur_tok);
			strncpy(mesh.Name, scur_tok, sizeof(mesh.Name) - 1);
		}
		else if (strcmp(scur_tok, "meshnormals") == 0)
		{
			curSection = SECT_NORMAL_VX;
		}
		else if (strcmp(scur_tok, "meshtexturecoords") == 0)
		{
			curSection = SECT_TEXTURE_COORD;
		}
		else if (strcmp(scur_tok, "material") == 0)
		{
			curSection = SECT_MATERIAL;
		}
		else
		{
			// continue processing current mesh / section

			switch (curSection)
			{
			case SECT_VERTEX:			// in vertex section

				if (vxCount < 1)	// haven't gotten vx cnt yet
				{
					vxCount = atoi(scur_tok);
				}
				else
				{
					VertexType vx;

					vx.P.x = atof(scur_tok);
					GetXFileToken(pFile, scur_tok); vx.P.y = atof(scur_tok);
					GetXFileToken(pFile, scur_tok); vx.P.z = atof(scur_tok);

					// default normals so files w/o them are not null
					vx.N = Normalize(vx.P);

					mesh.vVx.push_back(vx);

					if (mesh.vVx.size() >= vxCount)	// end of section
					{
						// i think you have to explicitly set the face section here,
						// because there is no "Face" identifier, it just starts after
						// the last vertex
						curSection = SECT_FACE;
						break;
					}
				}

				break;

			case SECT_FACE:		// in face section

				if (faceCount < 1)		// haven't gotten face cnt yet
				{
					faceCount = atoi(scur_tok);
				}
				else
				{
					int faceVxCnt =  atoi(scur_tok);	// # vertices current face (only 3 now)
					if (faceVxCnt != 3)
					{
						Logger::Write(__FUNCTION__, "Non-triangular face encountered.");
					}

					FaceType Face;
					GetXFileToken(pFile, scur_tok); Face.vx0 = atoi(scur_tok);
					GetXFileToken(pFile, scur_tok); Face.vx1 = atoi(scur_tok);
					GetXFileToken(pFile, scur_tok); Face.vx2 = atoi(scur_tok);
					mesh.vFace.push_back(Face);

					if (mesh.vFace.size() >= faceCount)		// next section
					{
						curSection = SECT_NONE;
						break;
					}
				}
				break;

			case SECT_NORMAL_VX:		// in normal section, vertex subsection

				if (normalCount < 1)	// haven't read count yet
				{
					normalCount = atoi(scur_tok);
					normalCtr = 0;
				}
				else
				{
					mesh.vVx[normalCtr].N.x = atof(scur_tok);
					GetXFileToken(pFile, scur_tok);
					mesh.vVx[normalCtr].N.y = atof(scur_tok);
					GetXFileToken(pFile, scur_tok);
					mesh.vVx[normalCtr].N.z = atof(scur_tok);
					++normalCtr;
				}

				if (normalCtr >= normalCount)	// end of section
				{
					// i think you have to explicitly set the face section here,
					// because there is no "Face" identifier, it just starts after
					// the last vertex
					curSection = SECT_NORMAL_FACE;
					break;
				}

				break;

			case SECT_TEXTURE_COORD:

				// for x files, or at least the ones i've seen,
				// there's a tx coord for every individual vx, 
				// not one for each face vertex
				if (textureCount < 1)	// haven't gotten count yet
				{
					textureCount = atoi(scur_tok);
				}
				else
				{
					if (textureCtr < textureCount)
					{
						float tu = (float)atof(scur_tok);
						GetXFileToken(pFile, scur_tok);
						float tv = (float)atof(scur_tok);

						mesh.vTex.push_back(TexCoord(tu, tv));
						++textureCtr;
					}
				}
				break;

			case SECT_MATERIAL:
				{
					Material mat;
					float r, g, b, a;
					mat.SetName(scur_tok);
					GetXFileToken(pFile, scur_tok); r = static_cast<float>(atof(scur_tok));			// faceColor, ColorRGBA
					GetXFileToken(pFile, scur_tok); g = static_cast<float>(atof(scur_tok));
					GetXFileToken(pFile, scur_tok); b = static_cast<float>(atof(scur_tok));
					GetXFileToken(pFile, scur_tok); a = static_cast<float>(atof(scur_tok));
					mat.SetAmbient(vec3(r, g, b));
					mat.SetDiffuse(vec3(r, g, b));
					float power = 0.0F;
					GetXFileToken(pFile, scur_tok); power = static_cast<float>(atof(scur_tok));		// power, float
					mat.SetShininess(power);
					GetXFileToken(pFile, scur_tok); r = static_cast<float>(atof(scur_tok));			// specularColor, ColorRGB
					GetXFileToken(pFile, scur_tok); g = static_cast<float>(atof(scur_tok));
					GetXFileToken(pFile, scur_tok); b = static_cast<float>(atof(scur_tok));
					mat.SetSpecular(vec3(r, g, b));
					GetXFileToken(pFile, scur_tok); r = static_cast<float>(atof(scur_tok));			// emissiveColor, ColorRGB
					GetXFileToken(pFile, scur_tok); g = static_cast<float>(atof(scur_tok));
					GetXFileToken(pFile, scur_tok); b = static_cast<float>(atof(scur_tok));
					//mat.SetSEmissive(vec3(r, g, b));
					GetXFileToken(pFile, scur_tok);													// TextureFileName { "crate512.jpg"; },	opt
					if (strcmp("texturefilename", scur_tok) == 0)
					{
						GetXFileToken(pFile, scur_tok);
						if (m_Texture.IsValid() == false)
						{
							mat.SetMapDiffuse(scur_tok);
							XString strTextureFile = scur_tok;
							strTextureFile = GetTextureFileName(scur_tok);
							if (m_Texture.Load(strTextureFile.c_str()) == false)
								Logger::Write(__FUNCTION__, "Unable to load texture file: %s", strTextureFile.c_str());
						}
					}
					mv_Mat.push_back(mat);

					// done with material
					curSection = SECT_NONE;
				}
				break;

			case SECT_NORMAL_FACE:		// unhandled cases
			case SECT_NONE:
				break;

			}
		}		// if scur_tok == "mesh"
	}			// while not eof

	// if have a mesh pending
	if (vxCount > 0)	
	{
		mv_Mesh.push_back(mesh);
	}

	// apply texture coordinates to faces
	for (int meshIndex = 0; meshIndex < GetMeshCount(); ++meshIndex)
	{
		MeshType* pMesh = &mv_Mesh[meshIndex];
		for (int faceIndex = 0; faceIndex < GetFaceCount(meshIndex); ++faceIndex)
		{
			pMesh->vFace[faceIndex].tx0 = pMesh->vFace[faceIndex].vx0;
			pMesh->vFace[faceIndex].tx1 = pMesh->vFace[faceIndex].vx1;
			pMesh->vFace[faceIndex].tx2 = pMesh->vFace[faceIndex].vx2;
		}
	}
	fclose(pFile);

	CalculateNormals();

	return true;
}

int Model_X::GetXFileToken(FILE* pFile, char* token)
{
	// Get a token from a text X file.
	//
	// Inputs:
	//		pFile = file handle
	//		token = points to an 80-byte string
	//
	// Outputs:
	//		token = points to next token
	//		return = # of bytes returned
	//

	char scur_tok[255] = { 0 };
	char ch;
	int nstr_ctr = 0;
	bool bComment = false;

	nstr_ctr = 0;
	while (!feof(pFile))
	{
		ch = fgetc(pFile);
		if (!strchr(X_DELIMITERS, ch))	// not a delimiter
		{
			bComment = false;			    // see if it's a comment
			if (ch == '/')
			{
				ch = fgetc(pFile);
				if (ch == '/')
					bComment = true;
			}
			else
			{
				if (ch == '#')
					bComment = true;
			}

			if (bComment)		// comment, scan to end of line
			{
				ch = fgetc(pFile);
				if (ch == '$')	// Fe command, scan for parameter
				{
					// not checking command now, only "$line:" supported
					while ((!feof(pFile)) && (ch != ':'))
						ch = fgetc(pFile);
					// just clear comment & continue processing as usual,
					// token returned will be parameter
					bComment = false;
				}
				else
				{
					while ((!feof(pFile)) && (ch != 10))
						ch = fgetc(pFile);
				}
			}
			else	// not a delimiter and not a comment, exit loop
			{
				break;
			}
		}
	}

	while (!feof(pFile))
	{
		if (!strchr(X_DELIMITERS, ch))
		{
			ch = tolower(ch);
			scur_tok[nstr_ctr++] = ch;
			scur_tok[nstr_ctr] = 0;
		}
		else
		{
			break;
		}
		ch = fgetc(pFile);
	}

	// copy the string on the stack to the system memory
	// location pointed to by "token"
	strcpy(token, scur_tok);
	return nstr_ctr;
}

bool Model_X::Save(const char* fileName)
{
	// Save file in .x text format.
	//

	bool bReturn = true;

	XString strFileName;
	if (fileName == nullptr)
		strFileName = ms_FileName;
	else
		strFileName = fileName;
	strFileName.Trim();

	FILE* pFile = fopen(strFileName.c_str(), "wt");
	if (!pFile)
	{
		Logger::Write(__FUNCTION__, "Unable to open file '%s'.", fileName);
		bReturn = false;
	}
	else
	{
		// clean up / minimize data

		DeleteEqualVertices();
		DeleteUnusedVertices();

		// write header

		fprintf(pFile, "xof 0302txt 0032\n");
		fprintf(pFile, "Header {\n");
		fprintf(pFile, "1;\n");
		fprintf(pFile, "0;\n");
		fprintf(pFile, "1;\n");
		fprintf(pFile, "}\n\n");

		// write meshes

		for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
		{
			int vxIdx = 0;
			int faceIdx = 0;

			int vxCount = static_cast<int>(mv_Mesh[meshIdx].vVx.size());

			// ensure mesh has a name
			XString strMeshName = mv_Mesh[meshIdx].Name;
			strMeshName.Trim();
			if (strMeshName.GetLength() == 0)
				strMeshName = strMeshName.Format("Mesh%d", meshIdx);
			fprintf(pFile, "Mesh %s {\n", strMeshName.c_str());

			// write vertices

			fprintf(pFile, "%u;\n", vxCount);			// vertex count

			for (vxIdx = 0; vxIdx < vxCount; vxIdx++)
			{
				if (vxIdx < vxCount - 1)
				{
					fprintf(pFile, "%.6f; %.6f; %.6f;,\n",
						mv_Mesh[meshIdx].vVx[vxIdx].P.x,
						mv_Mesh[meshIdx].vVx[vxIdx].P.y,
						mv_Mesh[meshIdx].vVx[vxIdx].P.z);
				}
				else
				{
					fprintf(pFile, "%.6f; %.6f; %.6f;;\n",
						mv_Mesh[meshIdx].vVx[vxIdx].P.x,
						mv_Mesh[meshIdx].vVx[vxIdx].P.y,
						mv_Mesh[meshIdx].vVx[vxIdx].P.z);
				}
			}
			fprintf(pFile, "\n");

			// write faces

			int faceCount = (int)mv_Mesh[meshIdx].vFace.size();
			fprintf(pFile, "%d;\n", faceCount);			// face count

			for (faceIdx = 0; faceIdx < faceCount; faceIdx++)
			{
				if (faceIdx < faceCount - 1)
				{
					fprintf(pFile, "3; %d,%d,%d;,\n",
						mv_Mesh[meshIdx].vFace[faceIdx].vx0,
						mv_Mesh[meshIdx].vFace[faceIdx].vx1,
						mv_Mesh[meshIdx].vFace[faceIdx].vx2);
				}
				else
				{
					fprintf(pFile, "3; %d,%d,%d;;\n",
						mv_Mesh[meshIdx].vFace[faceIdx].vx0,
						mv_Mesh[meshIdx].vFace[faceIdx].vx1,
						mv_Mesh[meshIdx].vFace[faceIdx].vx2);
				}
			}
			fprintf(pFile, "\n");

			// write normals (not implemented, normals calculated on the fly)

			// write texture coordinates

			int tcCount = GetTexCoordCount(meshIdx);
			fprintf(pFile, "MeshTextureCoords {\n");
			fprintf(pFile, "%d;\n", tcCount);

			for (int txIdx = 0; txIdx < tcCount; ++txIdx)
			{
				if (txIdx < tcCount - 1)
				{
					fprintf(pFile, "%.4f; %.4f;\n",
						mv_Mesh[meshIdx].vTex[txIdx].u,
						mv_Mesh[meshIdx].vTex[txIdx].v);
				}
				else
				{
					fprintf(pFile, "%.4f; %.4f;;\n",
						mv_Mesh[meshIdx].vTex[txIdx].u,
						mv_Mesh[meshIdx].vTex[txIdx].v);
				}
			}
			fprintf(pFile, "}\n");		// end texture coordinates

			fprintf(pFile, "}\n\n");	// end current mesh
		}

		fclose(pFile);
	}

	return bReturn;
}
