#ifndef TERRAINGL_H
#define TERRAINGL_H

#include <assert.h>

#include <vector>
#include <stack>

#include "Camera.h"
#include "GLManager.h"
#include "ShaderGL.h"			// OpenGL shader class
#include "VertexBufferGL.h"		// OpenGL vertex buffer manager class
#include "TextureGL.h"			// OpenGL texture class
#include "Sphere.h"				// OpenGL sphere class
#include "SkyBox.h"				// SkyBox class
#include "ScaleColor.h"
#include "Terrain.h"			// base class

class TerrainGL : public Terrain
{
public:

	TerrainGL(const char* mapFolder);
	~TerrainGL();

	void Render(glm::mat4& matModel, glm::mat4& matView, glm::mat4& matProj, Camera& camera,
				int defaultFBO, SizeType winSize, double nearPlane, double farPlane);

	void Save();							// save terrain model
	void Rebuild(RectType& rectRegion);		// update vertex buffers (vertex/normals only), textures
	void Rebuild();							// update entire terrain region

	void SetColorScale(XString scale);
	void SetTexture(XString file_name);

	// terrain texture access
	void SetPixel(UInt16 x, UInt16 y, PixelType pix);
	PixelType GetPixel(UInt16 x, UInt16 y);
	int GetTextureScale();
	SizeType GetTextureDim();

	// render states
	void SetRenderAxes(bool render) { mb_RenderAxes = render; }
	bool GetRenderAxes() { return mb_RenderAxes; }
	void SetRenderLights(bool render) { mb_RenderLights = render; }
	bool GetRenderLights() { return mb_RenderLights; }
	void SetRenderTerrain(bool render) { mb_RenderTerrain = render; }
	bool GetRenderTerrain() { return mb_RenderTerrain; }
	void SetRenderTileInfo(bool render) { mb_RenderTileInfo = render; }
	bool GetRenderTileInfo() { return mb_RenderTileInfo; }

	void SetRenderSky(SkyBox::Mode mode);
	SkyBox::Mode GetRenderSky();

	Light& GetLight() { return m_Light; }

	static const int GetTileSize() { return TILE_SIZE; }

protected:

	virtual void OnDataChanged() override;

private:

	static const int TILE_SIZE = 16;									// quads on a side (TILE_SIZE+1 vertices)
	static const int INDEX_COUNT = (TILE_SIZE)*(TILE_SIZE) * 6;			// # of indices in tile index buffer (2 tris / quad * TILE_SIZE quads per side)
	static const int VERTEX_COUNT = (TILE_SIZE + 1)*(TILE_SIZE + 1);	// # of vertices in 1 tile

	static const int SKY_VERTEX_COUNT = 56;
	static const int SKY_INDEX_COUNT = 108 * 3;

	// vertex component offsets/slots (gl calls it 'index')
	const int INDEX_P = 0;		// vertex position
	const int INDEX_N = 1;		// vertex normal
	const int INDEX_T = 2;		// texture coordinates
	const int INDEX_F = 3;		// flags
	const int INDEX_I = 4;		// vertex index

	// tiles allow for memory management (modifying only sub-regions),
	// rendering improvement (clipping), and potentially unlimited terrain
	// size

	struct TileType
	{
		UInt32 swRow;			// database row/col of sw corner of tile
		UInt32 swCol;
		VEC3 minExt;			// extents of this tile
		VEC3 maxExt;

		VertexBufferGL vb;

		TileType()
			: swRow(0)
			, swCol(0)
		{
		}

		~TileType()
		{
			Logger::Write(__FUNCTION__, "Freeing tile: %d, %d", swRow, swCol);
		}
	};

	std::vector<TileType*> mv_Tile;     // table of tiles

	TextureGL m_texTerrain;				// terrain texture
	ShaderGL m_shaderTerrain;			// terrain shader

	VertexBufferGL m_vbCubePC;			// reusable cube PC buffer
	VertexBufferGL m_vbSkirtPNC;		// vertex buffer for rendering skirting around sides

	SkyBox m_SkyBox;

	Light m_Light;						// single scene light
	Sphere m_SphereLight;				// light control sphere
	Material m_Material;				// primary terrain material
	Material m_MatSkirt;				// terrain skirt material

	ScaleColor* mp_ScaleColor;

	// render switches

	bool mb_RenderAxes;
	bool mb_RenderLights;
	bool mb_RenderTerrain;
	bool mb_RenderTileInfo;

private:

	void RenderAxes();
	void RenderBox(VEC3& minExt, VEC3& maxExt);
	void RenderContour();
	void RenderLights();
	void RenderScene(glm::mat4& matModel, glm::mat4& matView, glm::mat4& matProj, vec4& clipPlane);
	void RenderSkirt();
	void RenderTile(TileType* pTile);
	
	void LoadTerrain();
	void LoadTile(TileType* pTile, UInt32 tileRow, UInt32 tileCol);
	void FreeTiles();
	void LoadSkirt();
	VEC3 CalcVertexNormal(int vxRow, int vxCol);
	bool OnEdge(int vxRow, int vxCol);

	void CreateTexture();

	const unsigned int GetTileRowCount() { return GetRowCount() / TILE_SIZE; }
	const unsigned int GetTileColCount() { return GetColCount() / TILE_SIZE; }
};

#endif // #ifndef TERRAINGL_H
