/*
	Model_3DS.cpp

	TODO:
		- this thing leaks like a sieve  - free up all the "new"d memory

	3D Studio Model Class
	by: Matthew Fairfax

	This is a simple class for loading and viewing 3D Studio model files (.3ds). It supports models
	with multiple objects. It also supports multiple textures per object. It does not support the animation
	for 3D Studio models b/c there are simply too many ways for an artist to animate a 3D Studio model and
	I didn't want to impose huge limitations on the artists. However, I have imposed a limitation on how the
	models are textured:
		1) Every face must be assigned a material
		2) If you want the face to be textured assign the texture to the Diffuse Color map
		3) The texture must be supported by the GLTexture class which only supports bitmap and targa right now
		4) The texture must be located in the same directory as the model

	Support for non-textured faces is done by reading the color
	from the material's diffuse color.

	Some models have problems loading even if you follow all of the restrictions I have stated and
	I don't know why. If you can import the 3D Studio file into Milkshape 3D
	(http://www.swissquake.ch/chumbalum-soft) and then export it to a new 3D Studio file. This seems
	to fix many of the problems but there is a limit on the number of faces and vertices Milkshape 3D
	can read.
*/

#include "Model_3DS.h"

// The chunk's id numbers
const int MAIN3DS			= 0x4D4D;
const int MAIN_VERS			= 0x0002;
const int EDIT3DS			= 0x3D3D;
const int MESH_VERS			= 0x3D3E;
const int OBJECT			= 0x4000;
const int TRIG_MESH			= 0x4100;
const int VERT_LIST			= 0x4110;
const int FACE_DESC			= 0x4120;
const int FACE_MAT			= 0x4130;
const int TEX_VERTS			= 0x4140;
const int SMOOTH_GROUP		= 0x4150;
const int LOCAL_COORDS		= 0x4160;
const int MATERIAL			= 0xAFFF;
const int MAT_NAME			= 0xA000;
const int MAT_AMBIENT		= 0xA010;
const int MAT_DIFFUSE		= 0xA020;
const int MAT_SPECULAR		= 0xA030;
const int SHINY_PERC		= 0xA040;
const int SHINY_STR_PERC	= 0xA041;
const int TRANS_PERC		= 0xA050;
const int TRANS_FOFF_PERC	= 0xA052;
const int REF_BLUR_PERC		= 0xA053;
const int RENDER_TYPE		= 0xA100;
const int SELF_ILLUM		= 0xA084;
const int MAT_SELF_ILPCT	= 0xA08A;
const int WIRE_THICKNESS	= 0xA087;
const int MAT_TEXMAP		= 0xA200;
const int MAT_MAPNAME		= 0xA300;
const int ONE_UNIT			= 0x0100;
const int KEYF3DS			= 0xB000;
const int FRAMES			= 0xB008;
const int MESH_INFO			= 0xB002;
const int HIER_POS			= 0xB030;
const int HIER_FATHER		= 0xB010;
const int PIVOT_PT			= 0xB013;
const int TRACK00			= 0xB020;
const int TRACK01			= 0xB021;
const int TRACK02			= 0xB022;
const int COLOR_RGB			= 0x0010;
const int COLOR_TRU			= 0x0011;
const int COLOR_TRUG		= 0x0012;
const int COLOR_RGBG		= 0x0013;
const int PERC_INT			= 0x0030;
const int PERC_FLOAT		= 0x0031;

Model_3DS::Model_3DS()
	: Model()
	, mp_bin3ds(nullptr)
	, mp_Materials(nullptr)
	, mp_Objects(nullptr)
	, m_numObjects(0)
	, m_numMaterials(0)
	, m_totalVerts(0)
	, m_totalFaces(0)
{
}

Model_3DS::~Model_3DS()
{
	//delete mp_bin3ds;
	//delete mp_Materials;
}

bool Model_3DS::Load(const char* fileName)
{
	if (fileName == nullptr)
		return false;

	// holds the main chunk header
	ChunkHeader main;

	ms_FileName = fileName;
	ms_FileName.Trim();

	// strip quotes
	ms_FileName.Replace("\"", "");

	// Load the file
	mp_bin3ds = fopen(ms_FileName.c_str(), "rb");
	if (!mp_bin3ds)
		return false;

	// Make sure we are at the beginning
	fseek(mp_bin3ds, 0, SEEK_SET);

	// Load the Main Chunk's header
	fread(&main.id, sizeof(main.id), 1, mp_bin3ds);
	fread(&main.len, sizeof(main.len), 1, mp_bin3ds);

	// Start Processing
	MainChunkProcessor(main.len, ftell(mp_bin3ds));

	// Don't need the file anymore so close it
	fclose(mp_bin3ds);
	mp_bin3ds = nullptr;

	// Calculate the vertex normals
	CalculateNormals3DS();

	// Find the total number of faces and vertices
	m_totalFaces = 0;
	m_totalVerts = 0;

	for (int i = 0; i < m_numObjects; ++i)
	{
		m_totalFaces += mp_Objects[i].numFaces / 3;
		m_totalVerts += mp_Objects[i].numVerts;
	}

	// If the object doesn't have any texcoords generate some
	for (int k = 0; k < m_numObjects; ++k)
	{
		if (mp_Objects[k].numTexCoords == 0)
		{
			// Set the number of texture coords
			mp_Objects[k].numTexCoords = mp_Objects[k].numVerts;

			// Allocate an array to hold the texture coordinates
			mp_Objects[k].TexCoords = new GLfloat[mp_Objects[k].numTexCoords * 2];

			// Make some texture coords
			for (int m = 0; m < mp_Objects[k].numTexCoords; ++m)
			{
				mp_Objects[k].TexCoords[2 * m] = mp_Objects[k].Vertexes[3 * m];
				mp_Objects[k].TexCoords[2 * m + 1] = mp_Objects[k].Vertexes[3 * m + 1];
			}
		}
	}

	// Let's build simple colored textures for the materials w/o a texture
	for (int j = 0; j < m_numMaterials; ++j)
	{
		if (mp_Materials[j].textured == false)
		{
			//TODO:
			//			unsigned char r = mp_Materials[j].color.r;
			//			unsigned char g = mp_Materials[j].color.g;
			//			unsigned char b = mp_Materials[j].color.b;
			//			mp_Materials[j].tex.BuildColorTexture( r, g, b );
			//			mp_Materials[j].textured = true;
		}
	}

	// load base class structures for editing / rendering

	mv_Mesh.clear();
	for (int i = 0; i < m_numObjects; ++i)
	{
		MeshType mesh;
		const Object* pObject = &mp_Objects[i];
		strncpy(mesh.Name, pObject->name, sizeof(mesh.Name));
		for (int j = 0; j < pObject->numVerts; ++j)
		{
			VertexType vx;
			vx.P = VEC3(pObject->Vertexes[j * 3], pObject->Vertexes[j * 3 + 1], pObject->Vertexes[j * 3 + 2]);
			vx.N = VEC3(pObject->Normals[j * 3], pObject->Normals[j * 3 + 1], pObject->Normals[j * 3 + 2]);
			mesh.vVx.push_back(vx);
		}
		// object will always have texture coordinates because loader generates them
		// if they are missing (remove?)
		mesh.vTex.clear();
		for (int t = 0; t < pObject->numTexCoords; ++t)
		{
			TexCoord tx;
			tx.u = pObject->TexCoords[t * 2];
			tx.v = pObject->TexCoords[t * 2 + 1];
			mesh.vTex.push_back(tx);
		}
		// "numFaces" is # of face indices, not actual faces
		assert(pObject->numFaces % 3 == 0);
		int faceCount = pObject->numFaces / 3;
		for (int faceIdx = 0; faceIdx < faceCount; ++faceIdx)
		{
			FaceType face;
			face.vx0 = pObject->Faces[faceIdx * 3];
			face.vx1 = pObject->Faces[faceIdx * 3 + 1];
			face.vx2 = pObject->Faces[faceIdx * 3 + 2];
			face.tx0 = face.vx0;
			face.tx1 = face.vx1;
			face.tx2 = face.vx2;
			assert(face.vx0 < pObject->numVerts && face.vx1 < pObject->numVerts && face.vx2 < pObject->numVerts);
			mesh.vFace.push_back(face);
		}
		mesh.UpdateExtents();
		mv_Mesh.push_back(mesh);
	}
	mv_Mat.clear();
	for (int i = 0; i < m_numMaterials; ++i)
	{
		Material mat;
		mat.SetName(mp_Materials[i].name);
		mat.SetDiffuse(vec3(mp_Materials[i].color.GetR(), mp_Materials[i].color.GetG(), mp_Materials[i].color.GetB()));
		if (mp_Materials[i].textured)
		{
			// since per/object (or other groupings) textures are not
			// implemented, just grab the first texture and use for the 
			// entire model
			if (m_Texture.IsValid() == false)
			{
				XString strFileName = GetTextureFileName(mp_Materials[i].mapDiffuse);
				if (m_Texture.Load(strFileName.c_str()) == false)
					Logger::Write(__FUNCTION__, "Unable to load texture file: %s", strFileName.c_str());
			}
			mat.SetMapDiffuse(mp_Materials[i].mapDiffuse);
		}
		mv_Mat.push_back(mat);
	}

	return true;
}

void Model_3DS::CalculateNormals3DS()
{
	for (int i = 0; i < m_numObjects; ++i)
	{
		for (int g = 0; g < mp_Objects[i].numVerts; ++g)
		{
			// Reduce each vert's normal to unit

			VEC3 unit;

			unit.x = mp_Objects[i].Normals[g * 3];
			unit.y = mp_Objects[i].Normals[g * 3 + 1];
			unit.z = mp_Objects[i].Normals[g * 3 + 2];

			float length = sqrtf((float)(unit.x*unit.x + unit.y*unit.y + unit.z*unit.z));

			if (length == 0.0f)
				length = 1.0f;

			unit /= length;

			mp_Objects[i].Normals[g * 3] = (float)unit.x;
			mp_Objects[i].Normals[g * 3 + 1] = (float)unit.y;
			mp_Objects[i].Normals[g * 3 + 2] = (float)unit.z;
		}
	}
}

void Model_3DS::MainChunkProcessor(long length, long findex)
{
	ChunkHeader h;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	while (ftell(mp_bin3ds) < (findex + length - 6))
	{
		fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
		fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

		switch (h.id)
		{
		case EDIT3DS:
			// This is the mesh information like vertices, faces, and materials
			EditChunkProcessor(h.len, ftell(mp_bin3ds));
			break;
		case KEYF3DS:
			// I left this in case anyone gets very ambitious
			//KeyFrameChunkProcessor(h.len, ftell(mp_bin3ds));
			break;
		default:
			break;
		}

		fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::EditChunkProcessor(long length, long findex)
{
	ChunkHeader h;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	// First count the number of Objects and mp_Materials
	while (ftell(mp_bin3ds) < (findex + length - 6))
	{
		fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
		fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

		switch (h.id)
		{
		case OBJECT:
			++m_numObjects;
			break;
		case MATERIAL:
			++m_numMaterials;
			break;
		default:
			break;
		}

		fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
	}

	// Now load the materials
	if (m_numMaterials > 0)
	{
		mp_Materials = new Material3DS[m_numMaterials];

		// Material is set to untextured until we find otherwise
		for (int d = 0; d < m_numMaterials; d++)
			mp_Materials[d].textured = false;

		fseek(mp_bin3ds, findex, SEEK_SET);

		int i = 0;

		while (ftell(mp_bin3ds) < (findex + length - 6))
		{
			fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
			fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

			switch (h.id)
			{
			case MATERIAL:
				MaterialChunkProcessor(h.len, ftell(mp_bin3ds), i);
				i++;
				break;
			default:
				break;
			}

			fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
		}
	}

	// Load the Objects (individual meshes in the whole model)
	if (m_numObjects > 0)
	{
		mp_Objects = new Object[m_numObjects];

		// Set the textured variable to false until we find a texture
		for (int k = 0; k < m_numObjects; k++)
			mp_Objects[k].textured = false;

		// Zero the objects position and rotation
		for (int m = 0; m < m_numObjects; m++)
		{
			mp_Objects[m].pos.x = 0.0f;
			mp_Objects[m].pos.y = 0.0f;
			mp_Objects[m].pos.z = 0.0f;

			mp_Objects[m].rot.x = 0.0f;
			mp_Objects[m].rot.y = 0.0f;
			mp_Objects[m].rot.z = 0.0f;
		}

		// Zero out the number of texture coords
		for (int n = 0; n < m_numObjects; ++n)
			mp_Objects[n].numTexCoords = 0;

		fseek(mp_bin3ds, findex, SEEK_SET);

		int j = 0;

		while (ftell(mp_bin3ds) < (findex + length - 6))
		{
			fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
			fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

			switch (h.id)
			{
			case OBJECT:
				ObjectChunkProcessor(h.len, ftell(mp_bin3ds), j);
				j++;
				break;
			default:
				break;
			}

			fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
		}
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::MaterialChunkProcessor(long length, long findex, int matindex)
{
	ChunkHeader h;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	while (ftell(mp_bin3ds) < (findex + length - 6))
	{
		fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
		fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

		switch (h.id)
		{
		case MAT_NAME:
			// Loads the material's names
			MaterialNameChunkProcessor(h.len, ftell(mp_bin3ds), matindex);
			break;
		case MAT_AMBIENT:
			//ColorChunkProcessor( h.len, ftell(mp_bin3ds) );
			break;
		case MAT_DIFFUSE:
			DiffuseColorChunkProcessor(h.len, ftell(mp_bin3ds), matindex);
			break;
		case MAT_SPECULAR:
			//ColorChunkProcessor( h.len, ftell(mp_bin3ds) );
		case MAT_TEXMAP:
			// Finds the names of the textures of the material and loads them
			TextureMapChunkProcessor(h.len, ftell(mp_bin3ds), matindex);
			break;
		default:
			break;
		}

		fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::MaterialNameChunkProcessor(long /*length*/, long findex, int matindex)
{
	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	// Read the material's name
	for (int i = 0; i < sizeof(mp_Materials[0].name); i++)
	{
		mp_Materials[matindex].name[i] = fgetc(mp_bin3ds);
		if (mp_Materials[matindex].name[i] == 0)
			break;
	}

	// move the file pointer back to where we got it so that the ProcessChunk() which we 
	// interrupted will read from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::DiffuseColorChunkProcessor(long length, long findex, int matindex)
{
	ChunkHeader h;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	while (ftell(mp_bin3ds) < (findex + length - 6))
	{
		fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
		fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

		// Determine the format of the color and load it
		switch (h.id)
		{
		case COLOR_RGB:
			// A rgb float color chunk
			FloatColorChunkProcessor(h.len, ftell(mp_bin3ds), matindex);
			break;
		case COLOR_TRU:
			// A rgb int color chunk
			IntColorChunkProcessor(h.len, ftell(mp_bin3ds), matindex);
			break;
		case COLOR_RGBG:
			// A rgb gamma corrected float color chunk
			FloatColorChunkProcessor(h.len, ftell(mp_bin3ds), matindex);
			break;
		case COLOR_TRUG:
			// A rgb gamma corrected int color chunk
			IntColorChunkProcessor(h.len, ftell(mp_bin3ds), matindex);
			break;
		default:
			break;
		}

		fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::FloatColorChunkProcessor(long /*length*/, long findex, int matindex)
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	fread(&r, sizeof(r), 1, mp_bin3ds);
	fread(&g, sizeof(g), 1, mp_bin3ds);
	fread(&b, sizeof(b), 1, mp_bin3ds);

	mp_Materials[matindex].color.R = (unsigned char)(r*255.0);
	mp_Materials[matindex].color.G = (unsigned char)(g*255.0);
	mp_Materials[matindex].color.B = (unsigned char)(b*255.0);
	mp_Materials[matindex].color.A = 255;

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::IntColorChunkProcessor(long /*length*/, long findex, int matindex)
{
	unsigned char r;
	unsigned char g;
	unsigned char b;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	fread(&r, sizeof(r), 1, mp_bin3ds);
	fread(&g, sizeof(g), 1, mp_bin3ds);
	fread(&b, sizeof(b), 1, mp_bin3ds);

	mp_Materials[matindex].color.R = r;
	mp_Materials[matindex].color.G = g;
	mp_Materials[matindex].color.B = b;
	mp_Materials[matindex].color.A = 255;

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::TextureMapChunkProcessor(long length, long findex, int matindex)
{
	ChunkHeader h;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	while (ftell(mp_bin3ds) < (findex + length - 6))
	{
		fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
		fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

		switch (h.id)
		{
		case MAT_MAPNAME:
			// Read the name of texture in the Diffuse Color map
			MapNameChunkProcessor(h.len, ftell(mp_bin3ds), matindex);
			break;
		default:
			break;
		}

		fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::MapNameChunkProcessor(long /*length*/, long findex, int matindex)
{
	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	memset(mp_Materials[matindex].mapDiffuse, 0, sizeof(mp_Materials[matindex].mapDiffuse));

	// Read the name of the texture
	char name[80] = { 0 };
	for (int i = 0; i < sizeof(name); ++i)
	{
		name[i] = fgetc(mp_bin3ds);
		if (name[i] == 0)
			break;
	}

	// Load the material texture
	XString strFileName = GetTextureFileName(name);
//TODO:
//these per object textures are not currently in use
	if (mp_Materials[matindex].tex.Load(strFileName.c_str()) == false)
	{
		Logger::Write(__FUNCTION__, "Unable to load texture file: %s", strFileName.c_str());
	}
	else
	{
		strncpy(mp_Materials[matindex].mapDiffuse, name, strlen(name));
		mp_Materials[matindex].textured = true;
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::ObjectChunkProcessor(long length, long findex, int objindex)
{
	ChunkHeader h;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	// Load the object's name
	for (int i = 0; i < sizeof(mp_Objects[0].name); ++i)
	{
		mp_Objects[objindex].name[i] = fgetc(mp_bin3ds);
		if (mp_Objects[objindex].name[i] == 0)
			break;
	}

	while (ftell(mp_bin3ds) < (findex + length - 6))
	{
		fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
		fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

		switch (h.id)
		{
		case TRIG_MESH:
			// Process the triangles of the object
			TriangularMeshChunkProcessor(h.len, ftell(mp_bin3ds), objindex);
			break;
		default:
			break;
		}

		fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::TriangularMeshChunkProcessor(long length, long findex, int objindex)
{
	ChunkHeader h;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	while (ftell(mp_bin3ds) < (findex + length - 6))
	{
		fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
		fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

		switch (h.id)
		{
		case VERT_LIST:
			// Load the vertices of the onject
			VertexListChunkProcessor(h.len, ftell(mp_bin3ds), objindex);
			break;
		case LOCAL_COORDS:
			//LocalCoordinatesChunkProcessor(h.len, ftell(mp_bin3ds));
			break;
		case TEX_VERTS:
			// Load the texture coordinates for the vertices
			TexCoordsChunkProcessor(h.len, ftell(mp_bin3ds), objindex);
			mp_Objects[objindex].textured = true;
			break;
		default:
			break;
		}

		fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
	}

	// After we have loaded the vertices we can load the faces
	fseek(mp_bin3ds, findex, SEEK_SET);

	while (ftell(mp_bin3ds) < (findex + length - 6))
	{
		fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
		fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

		switch (h.id)
		{
		case FACE_DESC:
			// Load the faces of the object
			FacesDescriptionChunkProcessor(h.len, ftell(mp_bin3ds), objindex);
			break;
		default:
			break;
		}

		fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::VertexListChunkProcessor(long /*length*/, long findex, int objindex)
{
	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	// Read the number of vertices of the object
	unsigned short numVerts = 0;
	fread(&numVerts, sizeof(numVerts), 1, mp_bin3ds);

	// Allocate arrays for the vertices and normals
	mp_Objects[objindex].Vertexes = new GLfloat[numVerts * 3];
	mp_Objects[objindex].Normals = new GLfloat[numVerts * 3];

	// Assign the number of vertices for future use
	mp_Objects[objindex].numVerts = numVerts;

	// Zero out the normals array
	for (int j = 0; j < numVerts * 3; j++)
		mp_Objects[objindex].Normals[j] = 0.0f;

	// Read the vertices, switching the y and z coordinates and changing the sign of the z coordinate
	for (int i = 0; i < numVerts * 3; i += 3)
	{
		fread(&mp_Objects[objindex].Vertexes[i], sizeof(GLfloat), 1, mp_bin3ds);
		fread(&mp_Objects[objindex].Vertexes[i + 2], sizeof(GLfloat), 1, mp_bin3ds);
		fread(&mp_Objects[objindex].Vertexes[i + 1], sizeof(GLfloat), 1, mp_bin3ds);

		// Change the sign of the z coordinate
		mp_Objects[objindex].Vertexes[i + 2] = -mp_Objects[objindex].Vertexes[i + 2];
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::TexCoordsChunkProcessor(long /*length*/, long findex, int objindex)
{
	// The number of texture coordinates
	unsigned short numCoords;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	// Read the number of coordinates
	fread(&numCoords, sizeof(numCoords), 1, mp_bin3ds);

	// Allocate an array to hold the texture coordinates
	mp_Objects[objindex].TexCoords = new GLfloat[numCoords * 2];

	// Set the number of texture coords
	mp_Objects[objindex].numTexCoords = numCoords;

	// Read texture coordiantes into the array
	for (int i = 0; i < numCoords * 2; i += 2)
	{
		fread(&mp_Objects[objindex].TexCoords[i], sizeof(GLfloat), 1, mp_bin3ds);
		fread(&mp_Objects[objindex].TexCoords[i + 1], sizeof(GLfloat), 1, mp_bin3ds);
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::FacesDescriptionChunkProcessor(long length, long findex, int objindex)
{
	ChunkHeader h;
	unsigned short numFaces;	// The number of faces in the object
	unsigned short vertA;		// The first vertex of the face
	unsigned short vertB;		// The second vertex of the face
	unsigned short vertC;		// The third vertex of the face
	unsigned short flags;		// The winding order flags
	long subs;					// Holds our place in the file
	int numMatFaces = 0;		// The number of different materials

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	// Read the number of faces
	fread(&numFaces, sizeof(numFaces), 1, mp_bin3ds);

	// Allocate an array to hold the faces
	mp_Objects[objindex].Faces = new GLushort[numFaces * 3];
	// Store the number of faces
	mp_Objects[objindex].numFaces = numFaces * 3;

	// Read the faces into the array
	for (int i = 0; i < numFaces * 3; i += 3)
	{
		// Read the vertices of the face
		fread(&vertA, sizeof(vertA), 1, mp_bin3ds);
		fread(&vertB, sizeof(vertB), 1, mp_bin3ds);
		fread(&vertC, sizeof(vertC), 1, mp_bin3ds);
		fread(&flags, sizeof(flags), 1, mp_bin3ds);

		// Place them in the array
		mp_Objects[objindex].Faces[i] = vertA;
		mp_Objects[objindex].Faces[i + 1] = vertB;
		mp_Objects[objindex].Faces[i + 2] = vertC;

		// Calculate the face's normal
		VEC3 n;
		VEC3 v1;
		VEC3 v2;
		VEC3 v3;

		v1.x = mp_Objects[objindex].Vertexes[vertA * 3];
		v1.y = mp_Objects[objindex].Vertexes[vertA * 3 + 1];
		v1.z = mp_Objects[objindex].Vertexes[vertA * 3 + 2];
		v2.x = mp_Objects[objindex].Vertexes[vertB * 3];
		v2.y = mp_Objects[objindex].Vertexes[vertB * 3 + 1];
		v2.z = mp_Objects[objindex].Vertexes[vertB * 3 + 2];
		v3.x = mp_Objects[objindex].Vertexes[vertC * 3];
		v3.y = mp_Objects[objindex].Vertexes[vertC * 3 + 1];
		v3.z = mp_Objects[objindex].Vertexes[vertC * 3 + 2];

		// calculate the normal
		float u[3], v[3];

		// V2 - V3;
		u[0] = (float)(v2.x - v3.x);
		u[1] = (float)(v2.y - v3.y);
		u[2] = (float)(v2.z - v3.z);

		// V2 - V1;
		v[0] = (float)(v2.x - v1.x);
		v[1] = (float)(v2.y - v1.y);
		v[2] = (float)(v2.z - v1.z);

		n.x = (u[1] * v[2] - u[2] * v[1]);
		n.y = (u[2] * v[0] - u[0] * v[2]);
		n.z = (u[0] * v[1] - u[1] * v[0]);

		// Add this normal to its verts' normals
		mp_Objects[objindex].Normals[vertA * 3] += (float)n.x;
		mp_Objects[objindex].Normals[vertA * 3 + 1] += (float)n.y;
		mp_Objects[objindex].Normals[vertA * 3 + 2] += (float)n.z;
		mp_Objects[objindex].Normals[vertB * 3] += (float)n.x;
		mp_Objects[objindex].Normals[vertB * 3 + 1] += (float)n.y;
		mp_Objects[objindex].Normals[vertB * 3 + 2] += (float)n.z;
		mp_Objects[objindex].Normals[vertC * 3] += (float)n.x;
		mp_Objects[objindex].Normals[vertC * 3 + 1] += (float)n.y;
		mp_Objects[objindex].Normals[vertC * 3 + 2] += (float)n.z;
	}

	// Store our current file position
	subs = ftell(mp_bin3ds);

	// Check to see how many materials the faces are split into
	while (ftell(mp_bin3ds) < (findex + length - 6))
	{
		fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
		fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

		switch (h.id)
		{
		case FACE_MAT:
			//FacesMaterialsListChunkProcessor(h.len, ftell(mp_bin3ds), objindex);
			numMatFaces++;
			break;
		default:
			break;
		}

		fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
	}

	// Split the faces up according to their materials
	if (numMatFaces > 0)
	{
		// Allocate an array to hold the lists of faces divided by material
		mp_Objects[objindex].MatFaces = new MaterialFaces[numMatFaces];
		// Store the number of material faces
		mp_Objects[objindex].numMatFaces = numMatFaces;

		fseek(mp_bin3ds, subs, SEEK_SET);

		int j = 0;

		// Split the faces up
		while (ftell(mp_bin3ds) < (findex + length - 6))
		{
			fread(&h.id, sizeof(h.id), 1, mp_bin3ds);
			fread(&h.len, sizeof(h.len), 1, mp_bin3ds);

			switch (h.id)
			{
			case FACE_MAT:
				// Process the faces and split them up
				FacesMaterialsListChunkProcessor(h.len, ftell(mp_bin3ds), objindex, j);
				j++;
				break;
			default:
				break;
			}

			fseek(mp_bin3ds, (h.len - 6), SEEK_CUR);
		}
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}

void Model_3DS::FacesMaterialsListChunkProcessor(long /*length*/, long findex, int objindex, int subfacesindex)
{
	char name[80] = { 0 };		// material's name
	unsigned short numEntries;	// number of faces associated with this material
	int material;				// An index to the mp_Materials array for this material
	int i;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(mp_bin3ds, findex, SEEK_SET);

	// Read the material's name
	for (i = 0; i < sizeof(name); i++)
	{
		name[i] = fgetc(mp_bin3ds);
		if (name[i] == 0)
		{
			name[i] = NULL;
			break;
		}
	}

	// Faind the material's index in the mp_Materials array
	for (material = 0; material < m_numMaterials; material++)
	{
		if (strcmp(name, mp_Materials[material].name) == 0)
			break;
	}

	// Store this value for later so that we can find the material
	mp_Objects[objindex].MatFaces[subfacesindex].MatIndex = material;

	// Read the number of faces associated with this material
	fread(&numEntries, sizeof(numEntries), 1, mp_bin3ds);

	// Allocate an array to hold the list of faces associated with this material
	mp_Objects[objindex].MatFaces[subfacesindex].subFaces = new GLushort[numEntries * 3];
	// Store this number for later use
	mp_Objects[objindex].MatFaces[subfacesindex].numSubFaces = numEntries * 3;

	// Read the faces into the array
	for (i = 0; i < numEntries * 3; i += 3)
	{
		// read the face
		unsigned short face = 0;
		fread(&face, sizeof(face), 1, mp_bin3ds);
		// Add the face's vertices to the list
		mp_Objects[objindex].MatFaces[subfacesindex].subFaces[i] = mp_Objects[objindex].Faces[face * 3];
		mp_Objects[objindex].MatFaces[subfacesindex].subFaces[i + 1] = mp_Objects[objindex].Faces[face * 3 + 1];
		mp_Objects[objindex].MatFaces[subfacesindex].subFaces[i + 2] = mp_Objects[objindex].Faces[face * 3 + 2];
	}

	// move the file pointer back to where we got it so that the ProcessChunk() which we 
	// interrupted will read from the right place
	fseek(mp_bin3ds, findex, SEEK_SET);
}
