// TerrainGL.cpp
// Shader-based rendering subclass for terrain.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

// texture units for terrain pass
const int TERRAIN_TEX_UNIT = 0;

#include "Project.h"
#include "ShaderLib.h"
#include "TerrainGL.h"

#include <QImage>

TerrainGL::TerrainGL(const char* mapFolder)
	: Terrain(mapFolder)
	, mb_RenderAxes(true)
	, mb_RenderLights(false)
	, mb_RenderTerrain(true)
	, mb_RenderTileInfo(false)
	, mp_ScaleColor(nullptr)
{
	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	LoadTerrain();

	// create the light control sphere
	m_SphereLight.Create(VertexBufferGL::VertexType::PNC, 50.0);

	// create single terrain light & material
	m_Light.SetPos(-250.0F, 50.0f, 500.0F);
	GLManager::AddLight(0, &m_Light);

	m_Material.SetName("terrain");
	m_Material.SetAmbient(vec3(0.1f, 0.1f, 0.1f));
	m_Material.SetDiffuse(vec3(0.9f, 0.9f, 0.9f));
	m_Material.SetSpecular(vec3(0.1f, 0.1f, 0.1f));
	m_Material.SetShininess(100.0f);
	GLManager::AddMaterial(0, &m_Material);

	m_MatSkirt.SetName("skirt");
	m_MatSkirt.SetColor(vec3(0.6f, 0.4f, 0.2f));
	m_MatSkirt.SetSpecular(vec3(0.1f, 0.1f, 0.1f));
	m_MatSkirt.SetShininess(10.0f);
	GLManager::AddMaterial(1, &m_MatSkirt);

	// build terrain shader

	bool status = m_shaderTerrain.BuildShaderFromString("terrain", ShaderLib::GetVertexShaderTerrain(), ShaderLib::GetFragmentShaderTerrain());
	if (status == false)
	{
		Logger::Write(__FUNCTION__, "Terrain shader faild to build.");
		assert(false);
	}

	// shaders are added to static GLManager so can be accessed
	// from other classes
	GLManager::AddShader(m_shaderTerrain.GetName(), &m_shaderTerrain);

	// set the default lighting and material attributes
	m_shaderTerrain.SetUniform("Light.Color", vec3(1.0f, 1.0f, 1.0f));
	m_shaderTerrain.SetUniform("Light.Position", vec3(0.0f, 0.0f, 0.0f));
	m_shaderTerrain.SetUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
	m_shaderTerrain.SetUniform("Material.Kd", 0.9f, 0.9f, 0.9f);
	m_shaderTerrain.SetUniform("Material.Ks", 0.1f, 0.1f, 0.1f);
	m_shaderTerrain.SetUniform("Material.Shininess", 100.0f);

	// at end of construction, gl errors should be clear
	GLManager::CheckForOpenGLError(__FILE__, __LINE__);
}

TerrainGL::~TerrainGL()
{
	delete mp_ScaleColor;
}

void TerrainGL::Render(glm::mat4& matModel, glm::mat4& matView, glm::mat4& matProj, Camera& camera, 
					   int defaultFBO, SizeType winSize, double nearPlane, double farPlane)
{
	// enable first clipping plane in vertex shader
	glEnable(GL_CLIP_DISTANCE0);

	// this is what i need for "unbind" - put it back to the actual
	// default fbo, not opengl's, and set to window size
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
	glViewport(0, 0, winSize.cx, winSize.cy);

	// render scene
	// s/b able to just disable clipplane0, but not all drivers work, so 
	// send value way outside possible terrain limits
	glDisable(GL_CLIP_DISTANCE0);
	RenderScene(matModel, matView, matProj, vec4(0, 0, 1, -10000.0));

	RenderAxes();
	RenderContour();
	RenderLights();
	RenderSkirt();
}

void TerrainGL::RenderScene(glm::mat4& matModel, glm::mat4& matView, glm::mat4& matProj, vec4& clipPlane)
{
	// Render 3d terrain.
	//
	// Tried just passing camera in & calculating matrices here (seemed more convenient), but
	// too often the caller needs matrices for it's own rendering/calcs; think this is most logical -
	// let the rendering container take care of view/matrices, etc. and have terrain - like other
	// rendering components - just take matrices.
	//

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	// clear these for each call, there may be multiples per "paint" event
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// precomputed values to pass to shaders
	glm::mat4 matMVP = matProj * matView * matModel;	// precomputed model-view-projection matrix
	glm::mat4 matMV  = matView * matModel;				// precomputed model-view matrix

	// only set the ones the shader actually uses, otherwise you get
	// <variable> not found error
	GLManager::GetShader("PC")->SetUniform("MVP", matMVP);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	ShaderGL* pShader = GLManager::GetShader("terrain");
	pShader->SetUniform("ModelViewMatrix", matMV);
	pShader->SetUniform("NormalMatrix", mat3(vec3(matMV[0]), vec3(matMV[1]), vec3(matMV[2])));
	pShader->SetUniform("MVP", matMVP);

	pShader->SetUniform("Light.Color", vec3(m_Light.GetRed(), m_Light.GetGreen(), m_Light.GetBlue()));
	pShader->SetUniform("Light.Position", vec3(m_Light.GetPos().x, m_Light.GetPos().y, m_Light.GetPos().z));

	pShader->SetUniform("Material.Ka", m_Material.GetAmbient());
	pShader->SetUniform("Material.Kd", m_Material.GetDiffuse());
	pShader->SetUniform("Material.Ks", m_Material.GetSpecular());
	pShader->SetUniform("Material.Shininess", m_Material.GetShininess());

	// set horizontal clipping plane for water pre-rendering
	pShader->SetUniform("ClipPlane", clipPlane);

	// enable textures
	pShader->SetUniform("EnableTextures", true);
	// tell shader to use texture unit #0 for texture "Tex1"
	pShader->SetUniform("Tex1", m_texTerrain.GetTextureUnit());

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	pShader->Activate();
	m_texTerrain.Activate();

	if (mb_RenderTerrain)
	{
		GLManager::PushCull(true);
		GLManager::PushWind(GL_CW);
		GLManager::PushDepth(true, GL_LEQUAL);

		pShader->Activate();

		for (unsigned int i = 0; i < mv_Tile.size(); ++i)
			RenderTile(mv_Tile[i]);

		GLManager::PopDepth();
		GLManager::PopWind();
		GLManager::PopCull();
	}

	m_SkyBox.Render();

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);
}

void TerrainGL::RenderTile(TileType* pTile)
{
	pTile->vb.Render();

	// render wireframe box around tile and perhaps
	// other useful debugging info related to tile
	// and index structures
	if (mb_RenderTileInfo)
		RenderBox(pTile->minExt, pTile->maxExt);
}

void TerrainGL::RenderAxes()
{
	// Render Cartesian axes for entire world.

	if (mb_RenderAxes == false)
		return;

	float axisLength = GetRadius();

	VertexBufferGL::VertexPC vx;
	std::vector<VertexBufferGL::VertexPC> vxList;

	// render the XYZ axes according to application coordinate system, not OpenGL's
	vx.SetPos(-axisLength, 0, 0); vx.SetColor(0, 0, 0); vxList.push_back(vx);
	vx.SetPos(axisLength, 0, 0); vx.SetColor(1, 0, 0); vxList.push_back(vx);

	vx.SetPos(0, -axisLength, 0); vx.SetColor(0, 0, 0); vxList.push_back(vx);
	vx.SetPos(0, axisLength, 0); vx.SetColor(0, 1, 0); vxList.push_back(vx);

	vx.SetPos(0, 0, -axisLength); vx.SetColor(0, 0, 0); vxList.push_back(vx);
	vx.SetPos(0, 0, axisLength); vx.SetColor(0, 0, 1); vxList.push_back(vx);

	GLManager::GetShader("PC")->Activate();

	VertexBufferGL vbAxes;
	vbAxes.CreatePC(GL_LINES, vxList);
	vbAxes.Render();
}

void TerrainGL::RenderBox(VEC3& minExt, VEC3& maxExt)
{
	// Render a wireframe box - generally used for debugging

	float minx = (float)minExt.x; float miny = (float)minExt.y; float minz = (float)minExt.z;
	float maxx = (float)maxExt.x; float maxy = (float)maxExt.y; float maxz = (float)maxExt.z;

	float vx[24 * 3] = { 0 };
	float co[24 * 3] = { 0 };

	// color
	for (int i = 0; i < 24; ++i)
	{
		co[i * 3] = 0.1F;
		co[i * 3 + 1] = 0.1F;
		co[i * 3 + 2] = 0.9F;
	}

	// top
	vx[0] = minx; vx[1] = miny; vx[2] = maxz;
	vx[3] = minx; vx[4] = maxy; vx[5] = maxz;

	vx[6] = minx; vx[7] = maxy; vx[8] = maxz;
	vx[9] = maxx; vx[10] = maxy; vx[11] = maxz;

	vx[12] = maxx; vx[13] = maxy; vx[14] = maxz;
	vx[15] = maxx; vx[16] = miny; vx[17] = maxz;

	vx[18] = maxx; vx[19] = miny; vx[20] = maxz;
	vx[21] = minx; vx[22] = miny; vx[23] = maxz;

	// bottom
	vx[24] = minx; vx[25] = miny; vx[26] = minz;
	vx[27] = minx; vx[28] = maxy; vx[29] = minz;

	vx[30] = maxx; vx[31] = maxy; vx[32] = minz;
	vx[33] = minx; vx[34] = maxy; vx[35] = minz;

	vx[36] = maxx; vx[37] = maxy; vx[38] = minz;
	vx[39] = maxx; vx[40] = miny; vx[41] = minz;

	vx[42] = maxx; vx[43] = miny; vx[44] = minz;
	vx[45] = minx; vx[46] = miny; vx[47] = minz;

	// sides
	vx[48] = minx; vx[49] = miny; vx[50] = minz;
	vx[51] = minx; vx[52] = miny; vx[53] = maxz;

	vx[54] = minx; vx[55] = maxy; vx[56] = minz;
	vx[57] = minx; vx[58] = maxy; vx[59] = maxz;

	vx[60] = maxx; vx[61] = maxy; vx[62] = minz;
	vx[63] = maxx; vx[64] = maxy; vx[65] = maxz;

	vx[66] = maxx; vx[67] = miny; vx[68] = minz;
	vx[69] = maxx; vx[70] = miny; vx[71] = maxz;

	GLManager::GetShader("PC")->Activate();

	// create it
	if (m_vbCubePC.IsValid() == false)
	{
		// kind of a dumb thing i have to do to init
		// the buffer to the right size, even though 
		// no actually data is passed in vxList.
		std::vector<VertexBufferGL::VertexPC> vxList;
		vxList.resize(24);
		m_vbCubePC.CreatePC(GL_LINES, vxList);
	}

	// creating the buffer is slow, modifying its contents
	// is fast
	m_vbCubePC.ModifyPC(vx, co);
	m_vbCubePC.Render();
}

void TerrainGL::RenderContour()
{
	// Render contour lines.
	//

	if (mv_Contour.size() == 0)
		return;

	VertexBufferGL::VertexPC vx;
	std::vector<VertexBufferGL::VertexPC> vxList;

	GLManager::GetShader("PC")->Activate();

	// render the ordered line strips derived from post-processing
	for (int i = 0; i < mv_ContourLines.size(); ++i)
	{
		vx.SetColor(mv_ContourLines[i].color.GetR(), mv_ContourLines[i].color.GetG(), mv_ContourLines[i].color.GetB());

		vxList.clear();
		for (int j = 0; j < mv_ContourLines[i].pts.size(); ++j)
		{
			double x = mv_ContourLines[i].pts[j].x;
			double y = mv_ContourLines[i].pts[j].y;
			double height = mv_ContourLines[i].elev;

			vx.SetPos(x, y, height + 10.0);
			vxList.push_back(vx);
		}
		unsigned int lineType = GL_LINE_STRIP;
		if (mv_ContourLines[i].closed)
			lineType = GL_LINE_LOOP;

		VertexBufferGL vbAxes;
		vbAxes.CreatePC(lineType, vxList);
		vbAxes.Render();
	}

	// render the raw, unordered line segments
	//for (int k = 0; k < mv_Contour.size(); ++k)
	//{
	//	for (int i = 0; i < mv_Contour[k].size(); ++i)
	//	{
	//		double x0 = mv_Contour[k][i].x0;
	//		double y0 = mv_Contour[k][i].y0;
	//		double x1 = mv_Contour[k][i].x1;
	//		double y1 = mv_Contour[k][i].y1;
	//		double height = mv_ContourElev[k];

	//		vx.SetPos(x0, y0, height + 10.0);
	//		vxList.push_back(vx);

	//		vx.SetPos(x1, y1, height + 10.0);
	//		vxList.push_back(vx);
	//	}
	//}

	//GLManager::GetShader("PC")->Activate();
	//VertexBufferGL vbAxes;
	//vbAxes.CreatePC(GL_LINES, vxList);
	//vbAxes.Render();
}

void TerrainGL::RenderLights()
{
	// Show light sources and directions.
	//

	if (mb_RenderLights == false)
		return;

	VertexBufferGL::VertexPC vx;
	std::vector<VertexBufferGL::VertexPC> vxList;

	// draw ray from light to terrain origin

	vx.r = m_SphereLight.GetVertexPNC(0)->r; 
	vx.g = m_SphereLight.GetVertexPNC(0)->g; 
	vx.b = m_SphereLight.GetVertexPNC(0)->r;

	vx.x = m_Light.GetPos().x;
	vx.y = m_Light.GetPos().y;
	vx.z = m_Light.GetPos().z;
	vxList.push_back(vx);

	vx.x = 0;
	vx.y = 0;
	vx.z = 0;
	vxList.push_back(vx);

	VertexBufferGL vb;
	vb.CreatePC(GL_LINES, vxList);
	vb.Render();

	mat4 matModel = GLManager::GetMatModel();
	mat4 matView = GLManager::GetMatView();
	mat4 matProj = GLManager::GetMatProj();

	// translate the sphere model to the light position
	matModel = glm::translate(glm::vec3(m_Light.GetPos().x, m_Light.GetPos().y, m_Light.GetPos().z));
	glm::mat4 matMVP = matProj * matView * matModel;

	GLManager::GetShader("PNC")->Activate();
	GLManager::GetShader("PNC")->SetUniform("MVP", matMVP);

	// draw sphere representing light source
	m_SphereLight.Render();
}

void TerrainGL::RenderSkirt()
{
	// Render skirting around side of model.
	//

	if (m_vbSkirtPNC.IsValid() == false)
		return;

	GLManager::PushWind(GL_CW);

	ShaderGL* pShader = GLManager::GetShader("PNC");
	if (pShader)
	{
		pShader->Activate();
		pShader->SetUniform("Material.Ka", m_MatSkirt.GetAmbient());
		pShader->SetUniform("Material.Kd", m_MatSkirt.GetDiffuse());
		pShader->SetUniform("Material.Ks", m_MatSkirt.GetSpecular());
		pShader->SetUniform("Material.Shininess", m_MatSkirt.GetShininess());
	}

	m_vbSkirtPNC.Render();

	GLManager::PopWind();
}

void TerrainGL::Save()
{
	// Save entire terrain model.
	// 
	// To save elevation data only, see Terrain::SaveData().
	//

	if (IsDirty())
		SaveData();

	if (m_texTerrain.IsDirty())
		m_texTerrain.Save();
}

void TerrainGL::Rebuild()
{
	// Convenience overload that update entire
	// terrain extents.

	Rebuild(RectType(0, 0, GetColCount() - 1, GetRowCount() - 1));
}

void TerrainGL::Rebuild(RectType& rectRegion)
{
	// Update vertex buffers & textures.
	//
	// Inputs:
	//		(row0,col0)-(row1,col1) = region of terrain to reload
	//
	// Height and normals are updated. Indices and x/y 
	// do not change.
	//

	for (int i = 0; i < mv_Tile.size(); ++i)
	{
		// if tile intersects rectRegion

		if ((int)mv_Tile[i]->swCol > rectRegion.x1)
			continue;
		if ((int)(mv_Tile[i]->swCol + TILE_SIZE) < rectRegion.x0)
			continue;
		if ((int)mv_Tile[i]->swRow > rectRegion.y1)
			continue;
		if ((int)(mv_Tile[i]->swRow + TILE_SIZE) < rectRegion.y0)
			continue;

		float vx[VERTEX_COUNT * 3] = { 0 };
		float n[VERTEX_COUNT * 3] = { 0 };

		int destVx = 0;
		for (int r = 0; r <= TILE_SIZE; ++r)
		{
			for (int c = 0; c <= TILE_SIZE; ++c)
			{
				int dbRow = mv_Tile[i]->swRow + r;
				int dbCol = mv_Tile[i]->swCol + c;

				float x = dbCol * GetPitch();
				float y = dbRow * GetPitch();

				VEC3 N = CalcVertexNormal(dbRow, dbCol);
				double height = GetHeight(dbRow, dbCol);

				vx[destVx] = x;
				vx[destVx + 1] = y;
				vx[destVx + 2] = height;

				n[destVx] = N.x;
				n[destVx + 1] = N.y;
				n[destVx + 2] = N.z;

				destVx += 3;
			}
		}

		mv_Tile[i]->vb.ModifyPN(vx, n);
	}

	// rebind texture on presumption that if terrain was modified,
	// so was texture
	m_texTerrain.Update();

	// update skirt
	LoadSkirt();
}

PixelType TerrainGL::GetPixel(UInt16 x, UInt16 y)
{
	// Get a pixel from terrain texture.
	//
	// assume:
	//		- single texture (m_texTerrain)
	//		- write to image file index 0, read from index 1

	return m_texTerrain.GetPixel(x, y);
}

void TerrainGL::SetPixel(UInt16 x, UInt16 y, PixelType pix)
{
	// Set a pixel in terrain texture.
	//
	// assume:
	//		- single texture (m_texTerrain)
	//		- write to image file index 0, read from index 1

	m_texTerrain.SetPixel(x, y, pix);
}

SizeType TerrainGL::GetTextureDim()
{
	SizeType sz;
	sz.cx = m_texTerrain.GetWidth();
	sz.cy = m_texTerrain.GetHeight();
	return sz;
}

int TerrainGL::GetTextureScale()
{
	// Return # of texture pixels per data point.
	//

	int texScale = m_texTerrain.GetWidth() / GetColCount();

	if (texScale < 1)
		texScale = 1;

	return texScale;
}

void TerrainGL::SetColorScale(XString scale)
{
	// Use color scale for terrain color.
	//

	delete mp_ScaleColor;
	mp_ScaleColor = new ScaleColor(scale.c_str());

	CreateTexture();
}

void TerrainGL::SetTexture(XString file_name)
{
	// Use texture for terrain color.
	//

	delete mp_ScaleColor;
	mp_ScaleColor = nullptr;

	// since actual orthophoto can be extremely large,
	// try using orthophoto preview from report

	ms_TextureFile = XString::CombinePath(tree.report_path, "ortho.png");
	if (!m_texTerrain.Load(ms_TextureFile.c_str()))
	{
		Logger::Write(__FUNCTION__, "Unable to load texture: '%s'", ms_TextureFile.c_str());
	}

	m_texTerrain.Update();
}

void TerrainGL::CreateTexture()
{
	// for historical reasons, TextureGL requires a backing image file,
	// so lets create one here

	ms_TextureFile = XString::CombinePath(tree.dem_path, "dxm.png");
	QImage* pImage = new QImage(GetColCount(), GetRowCount(), QImage::Format::Format_ARGB32);

	for (int r = 0; r < GetRowCount(); ++r)
	{
		for (int c = 0; c < GetColCount(); ++c)
		{
			double sf = (GetHeight(r, c) - GetMinElev()) / (GetMaxElev() - GetMinElev());
			PixelType pix;
			if (mp_ScaleColor)
				pix = mp_ScaleColor->GetColorByHeight(sf);
			else
				pix.SetGraySF(sf);

			pImage->setPixel(c, GetRowCount() - r - 1, pix.GetRGBA());
		}
	}

	pImage->save(ms_TextureFile.c_str());
	delete pImage;

	if (!m_texTerrain.Load(ms_TextureFile.c_str()))
	{
		Logger::Write(__FUNCTION__, "Unable to load texture: '%s'", ms_TextureFile.c_str());
	}

	m_texTerrain.Update();
}

void TerrainGL::LoadTerrain()
{
	// Load rendering data for terrain model.
	//

	if (GetTileRowCount() < 1 || GetTileColCount() < 1)
	{
		Logger::Write(__FUNCTION__, "Terrain too small to render. Dimensions must be at least %d x %d data points.", TILE_SIZE, TILE_SIZE);
		return;
	}

	FreeTiles();

	for (UInt16 tileRow = 0; tileRow < GetTileRowCount(); ++tileRow)
	{
		for (UInt16 tileCol = 0; tileCol < GetTileColCount(); ++tileCol)
		{
			TileType* pTile = new TileType();
			LoadTile(pTile, tileRow, tileCol);
			mv_Tile.push_back(pTile);
		}
	}

	// Load texture file

	CreateTexture();

	// load skirt

	LoadSkirt();
}

void TerrainGL::LoadTile(TileType* pTile, UInt32 tileRow, UInt32 tileCol)
{
	// Populate the pTile data structure.
	//

	// convert tile indices to data point indices
	UInt32 swRow = tileRow * TILE_SIZE;
	UInt32 swCol = tileCol * TILE_SIZE;

	double swX = 0.0;
	double swY = 0.0;
	RowColToXY(swRow, swCol, swX, swY);

	double neX = 0.0;
	double neY = 0.0;
	RowColToXY(swRow + TILE_SIZE, swCol + TILE_SIZE, neX, neY);

	TerrainVertexDiskType vxDesc = GetVertex(swRow, swCol);
	pTile->minExt = VEC3(swX, swY, vxDesc.Height);
	pTile->maxExt = VEC3(neX, neY, vxDesc.Height);

	std::vector<VertexBufferGL::VertexPNTF> vxList;
	vxList.reserve(VERTEX_COUNT);

	std::vector<unsigned int> indexList;
	indexList.reserve(INDEX_COUNT);

	// Rows start at bottom and grow up
	for (int r = 0; r <= TILE_SIZE; ++r)
	{
		for (int c = 0; c <= TILE_SIZE; ++c)
		{
			// calc row/col relative to entire terrain
			UInt16 trnRow = swRow + r;
			UInt16 trnCol = swCol + c;

			TerrainVertexDiskType vxDesc = GetVertex(trnRow, trnCol);
			VEC3 N = CalcVertexNormal(trnRow, trnCol);

			VertexBufferGL::VertexPNTF vx;
			vx.x = static_cast<float>(trnCol * GetPitch());
			vx.y = static_cast<float>(trnRow * GetPitch());
			vx.z = static_cast<float>(GetHeight(trnRow, trnCol));
			vx.nx = static_cast<float>(N.x);
			vx.ny = static_cast<float>(N.y);
			vx.nz = static_cast<float>(N.z);
			vx.tu = (float)trnCol / (float)(GetColCount() - 1);
			vx.tv = 1.0F - (float)(trnRow) / (float)(GetRowCount() - 1);
			vx.flags = vxDesc.Flags;
			vxList.push_back(vx);

			if (pTile->minExt.z > vx.z)
				pTile->minExt.z = vx.z;
			if (pTile->maxExt.z < vx.z)
				pTile->maxExt.z = vx.z;

			if (r < TILE_SIZE && c < TILE_SIZE)
			{
				// calculate indices, assumes cw
				// winding order

				int v0 = r       * (TILE_SIZE + 1) + c;
				int v1 = (r + 1) * (TILE_SIZE + 1) + c;
				int v2 = (r + 1) * (TILE_SIZE + 1) + c + 1;
				int v3 = r       * (TILE_SIZE + 1) + c + 1;

				// NW triangle
				indexList.push_back(v0);
				indexList.push_back(v1);
				indexList.push_back(v2);

				// SE triangle
				indexList.push_back(v0);
				indexList.push_back(v2);
				indexList.push_back(v3);
			}
		}
	}

	if (pTile->vb.CreatePNTF(GL_TRIANGLES, vxList, indexList) == false)
	{
		Logger::Write(__FUNCTION__, "CreateVertexBuffer() failed");
	}

	pTile->swRow = swRow;
	pTile->swCol = swCol;
}

void TerrainGL::FreeTiles()
{
	for (auto tile : mv_Tile)
	{
		tile->vb.Delete();

		delete tile;
	}
	mv_Tile.clear();
}

void TerrainGL::LoadSkirt()
{
	std::vector<VertexBufferGL::VertexPNC> vxList;

	VertexBufferGL::VertexPNC vx;
	vx.SetColor(0.6F);

	// ensure base of skirt below terrain
	double base = GetMinElev() - 100.0;

	// create faces - front

	vx.SetNormal(0.0, -1.0, 0.0);
	for (int c = 0; c < GetColCount() - 1; ++c)
	{
		double x = c * GetPitch();
		double z0 = GetHeight(0, c);
		double z1 = GetHeight(0, c + 1);

		// NW triangle
		vx.SetPos(x, 0.0, z0); vxList.push_back(vx);
		vx.SetPos(x + GetPitch(), 0.0, z1); vxList.push_back(vx);
		vx.SetPos(x, 0.0, base); vxList.push_back(vx);

		// SE triangle
		vx.SetPos(x + GetPitch(), 0.0, z1); vxList.push_back(vx);
		vx.SetPos(x + GetPitch(), 0.0, base); vxList.push_back(vx);
		vx.SetPos(x, 0.0, base); vxList.push_back(vx);
	}

	// create faces - back

	vx.SetNormal(0.0, 1.0, 0.0);
	double y = (GetRowCount() - 1) * GetPitch();
	for (int c = 0; c < GetColCount() - 1; ++c)
	{
		double x = c * GetPitch();
		double z0 = GetHeight(GetRowCount() - 1, c + 1);
		double z1 = GetHeight(GetRowCount() - 1, c);

		// NW triangle
		vx.SetPos(x + GetPitch(), y, z0); vxList.push_back(vx);
		vx.SetPos(x, y, z1); vxList.push_back(vx);
		vx.SetPos(x + GetPitch(), y, base); vxList.push_back(vx);

		// SE triangle
		vx.SetPos(x, y, z1); vxList.push_back(vx);
		vx.SetPos(x, y, base); vxList.push_back(vx);
		vx.SetPos(x + GetPitch(), y, base); vxList.push_back(vx);
	}

	// create faces - side (+X)

	vx.SetNormal(-1.0, 0.0, 0.0);
	double x = (GetColCount() - 1) * GetPitch();
	for (int r = 0; r < GetRowCount() - 1; ++r)
	{
		double y = r * GetPitch();
		double z0 = GetHeight(r, GetColCount() - 1);
		double z1 = GetHeight(r + 1, GetColCount() - 1);

		// NW triangle
		vx.SetPos(x, y, z0);  vxList.push_back(vx);
		vx.SetPos(x, y + GetPitch(), z1);  vxList.push_back(vx);
		vx.SetPos(x, y, base); vxList.push_back(vx);

		// SE triangle
		vx.SetPos(x, y + GetPitch(), z1);  vxList.push_back(vx);
		vx.SetPos(x, y + GetPitch(), base); vxList.push_back(vx);
		vx.SetPos(x, y, base); vxList.push_back(vx);
	}

	// create faces - side (-X)

	vx.SetNormal(1.0, 0.0, 0.0);
	for (int r = 0; r < GetRowCount() - 1; ++r)
	{
		double y = r * GetPitch();
		double z0 = GetHeight(r + 1, 0);
		double z1 = GetHeight(r, 0);

		// NW triangle
		vx.SetPos(0, y + GetPitch(), z0);  vxList.push_back(vx);
		vx.SetPos(0, y, z1);  vxList.push_back(vx);
		vx.SetPos(0, y + GetPitch(), base); vxList.push_back(vx);

		// SE triangle
		vx.SetPos(0, y, z1);  vxList.push_back(vx);
		vx.SetPos(0, y, base); vxList.push_back(vx);
		vx.SetPos(0, y + GetPitch(), base); vxList.push_back(vx);
	}

	if (m_vbSkirtPNC.IsValid() == true)
	{
		// create temp buffers so i can modify vb
		// rather than delete/recreate

		std::vector<float>v(vxList.size()*3);
		std::vector<float>n(vxList.size()*3);

		for (int i = 0; i < vxList.size(); ++i)
		{
			v[i*3] = vxList[i].x;
			v[i*3 + 1] = vxList[i].y;
			v[i*3 + 2] = vxList[i].z;
			n[i*3] = vxList[i].nx;
			n[i*3 + 1] = vxList[i].ny;
			n[i*3 + 2] = vxList[i].nz;
		}

		m_vbSkirtPNC.ModifyPN(&v[0], &n[0]);
	}
	else
	{
		bool status = m_vbSkirtPNC.CreatePNC(GL_TRIANGLES, vxList);
		if (status == false)
		{
			Logger::Write(__FUNCTION__, "Unable to create skirt buffer.");
		}
	}
}

VEC3 TerrainGL::CalcVertexNormal(int row, int col)
{
	// Calculate vertex normal for vertex at terrain
	// row, col.
	//
	// Treat each vertex as center of triangle fan.
	//

	VEC3 N(0, 0, 1);	// return value

	if (OnEdge(row, col) == false)
	{
		TerrainVertexDiskType vx = GetVertex(row, col);

		TerrainVertexDiskType vxNW = GetVertex(row + 1, col - 1);
		TerrainVertexDiskType vxN = GetVertex(row + 1, col);
		TerrainVertexDiskType vxNE = GetVertex(row + 1, col + 1);
		TerrainVertexDiskType vxE = GetVertex(row, col + 1);
		TerrainVertexDiskType vxSE = GetVertex(row - 1, col + 1);
		TerrainVertexDiskType vxS = GetVertex(row - 1, col);
		TerrainVertexDiskType vxSW = GetVertex(row - 1, col - 1);
		TerrainVertexDiskType vxW = GetVertex(row, col - 1);

		VEC3 v = VEC3(col * GetPitch(), row * GetPitch(), vx.Height);

		VEC3 vNW = VEC3((col - 1) * GetPitch(), (row + 1) * GetPitch(), vxNW.Height);
		VEC3 vN = VEC3((col)* GetPitch(), (row + 1) * GetPitch(), vxN.Height);
		VEC3 vNE = VEC3((col + 1) * GetPitch(), (row + 1) * GetPitch(), vxNE.Height);
		VEC3 vE = VEC3((col + 1) * GetPitch(), (row)* GetPitch(), vxE.Height);
		VEC3 vSE = VEC3((col + 1) * GetPitch(), (row - 1) * GetPitch(), vxSE.Height);
		VEC3 vS = VEC3((col)* GetPitch(), (row - 1) * GetPitch(), vxS.Height);
		VEC3 vSW = VEC3((col - 1) * GetPitch(), (row - 1) * GetPitch(), vxSW.Height);
		VEC3 vW = VEC3((col - 1) * GetPitch(), (row)* GetPitch(), vxW.Height);

		VEC3 N0 = Normalize(CrossProduct(vN - v, vNW - v));
		VEC3 N1 = Normalize(CrossProduct(vNE - v, vN - v));
		VEC3 N2 = Normalize(CrossProduct(vE - v, vNE - v));
		VEC3 N3 = Normalize(CrossProduct(vSE - v, vE - v));
		VEC3 N4 = Normalize(CrossProduct(vS - v, vSE - v));
		VEC3 N5 = Normalize(CrossProduct(vSW - v, vS - v));
		VEC3 N6 = Normalize(CrossProduct(vW - v, vSW - v));
		VEC3 N7 = Normalize(CrossProduct(vNW - v, vW - v));

		N = (N0 + N1 + N2 + N3 + N4 + N5 + N6 + N7) * 0.125;
		N = Normalize(N);
	}

	return N;
}

bool TerrainGL::OnEdge(int vxRow, int vxCol)
{
	if ((vxRow == 0) || (vxCol == 0))
		return true;
	if ((vxRow == GetRowCount() - 1) || (vxCol == GetColCount() - 1))
		return true;

	return false;
}

void TerrainGL::SetRenderSky(SkyBox::Mode mode)
{
	m_SkyBox.SetMode(mode);
}

SkyBox::Mode TerrainGL::GetRenderSky()
{
	return m_SkyBox.GetMode();
}

void TerrainGL::OnDataChanged()
{
	// happens any time permanent terrain property changes

	__super::OnDataChanged();
}
