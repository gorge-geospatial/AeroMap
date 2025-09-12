#ifndef MODEL_3DS_H
#define MODEL_3DS_H

#include <stdio.h>

#include "Calc.h"
#include "TextureGL.h"
#include "Model.h"

class Model_3DS : public Model
{
public:

	// construction / destruction
	Model_3DS();
	virtual ~Model_3DS();

	// Model
	virtual Model::Format GetFormat() override { return Model::Format::ThreeDS; }
	virtual bool Load(const char* fileName) override;		// Loads a 3ds model

private:

	// Holds the material info
	// TODO: add color support for non textured polys
	struct Material3DS
	{
		char name[80];			// The material's name
		char mapDiffuse[255];	// Diffuse texture map file name
		TextureGL tex;			// The texture
		bool textured;			// whether or not it is textured
		PixelType color;		// Color struct holds the diffuse color of the material
	};

	// Every chunk in the 3ds file starts with this struct
	struct ChunkHeader
	{
		unsigned short id;		// chunk id
		unsigned long  len;		// chunk length
	};

	// I sort the mesh by material so that I won't have to switch textures a great deal
	struct MaterialFaces
	{
		unsigned short* subFaces;	// Index to our vertex array of all the faces that use this material
		int numSubFaces;			// The number of faces
		int MatIndex;				// An index to our materials
	};

	// The 3ds file can be made up of several objects
	struct Object
	{
		char name[80];				// object name
		float* Vertexes;			// array of vertices
		float* Normals;				// array of the normals for the vertices
		float* TexCoords;			// array of texture coordinates for the vertices
		unsigned short* Faces;		// array of face indices
		int numFaces;				// number of faces
		int numMatFaces;			// number of differnet material faces
		int numVerts;				// number of vertices
		int numTexCoords;			// number of vertices
		bool textured;				// True: the object has textures
		MaterialFaces* MatFaces;	// faces are divided by materials
		VEC3 pos;					// position to move the object to
		VEC3 rot;					// angles to rotate the object
	};

	int m_numObjects;			// total number of objects in the model
	int m_numMaterials;			// total number of materials in the model
	int m_totalVerts;			// total number of vertices in the model
	int m_totalFaces;			// total number of faces in the model
	Material3DS* mp_Materials;	// array of materials
	Object* mp_Objects;			// array of objects in the model
	FILE* mp_bin3ds;			// binary 3ds file

private:

	void IntColorChunkProcessor(long length, long findex, int matindex);
	void FloatColorChunkProcessor(long length, long findex, int matindex);

	void MainChunkProcessor(long length, long findex);	// Processes the Main Chunk that all the other chunks exist is
	void EditChunkProcessor(long length, long findex);	// Processes the model's info

	void MaterialChunkProcessor(long length, long findex, int matindex);		// Process materials
	void MaterialNameChunkProcessor(long length, long findex, int matindex);	// Process names of the materials
	void DiffuseColorChunkProcessor(long length, long findex, int matindex);	// Process material's diffuse color
	void TextureMapChunkProcessor(long length, long findex, int matindex);		// Process material's texture maps
	void MapNameChunkProcessor(long length, long findex, int matindex);			// Process names of the textures and load the textures

	void ObjectChunkProcessor(long length, long findex, int objindex);			// Process geometry
	void TriangularMeshChunkProcessor(long length, long findex, int objindex);	// Process triangles of the model
	void VertexListChunkProcessor(long length, long findex, int objindex);		// Process vertices of the model and loads them
	void TexCoordsChunkProcessor(long length, long findex, int objindex);		// Process texture cordiantes of the vertices and loads them
	void FacesDescriptionChunkProcessor(long length, long findex, int objindex);	// Process faces of the model and loads the faces
	void FacesMaterialsListChunkProcessor(long length, long findex, int objindex, int subfacesindex);	// Process materials of the faces and splits them up by material

	// Calculates the normals of the vertices by averaging
	// the normals of the faces that use that vertex
	void CalculateNormals3DS();
};

#endif // #ifndef MODEL_3DS_H
