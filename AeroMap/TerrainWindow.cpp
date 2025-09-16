// TerrainWindow.cpp
// Terrain model editor.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "ContourDlg.h"			// contouring options dialog
#include "ElevationDlg.h"		// change terrain elevation globally
#include "MetaDataDlg.h"

#include "ShaderLib.h"
#include "PngFile.h"
#include "GLManager.h"
#include "MainWindow.h"
#include "TerrainWindow.h"

#include <QProgressDialog>
#include <QStatusBar>

TerrainWindow::TerrainWindow(QWidget* parent, int model_type)
	: QOpenGLWidget(parent, Qt::CustomizeWindowHint)
	, mp_Terrain(nullptr)
	, mb_RenderDim(false)
	, mb_Selecting(false)
	, mb_DebugInfo(false)
	, mp_actionViewTop(nullptr)
	, mp_actionViewFront(nullptr)
	, mp_actionViewSide(nullptr)
	, mp_actionView3D(nullptr)
	, mp_actionFillSolid(nullptr)
	, mp_actionFillWire(nullptr)
	, mp_actionRenderAxes(nullptr)
	, mp_actionRenderDim(nullptr)
	, mp_actionRenderLights(nullptr)
	, mp_actionRenderSkySphere(nullptr)
	, mp_actionRenderSkyBox(nullptr)
	, mp_actionRenderSkyNone(nullptr)
	, mp_actionRenderTerrain(nullptr)
	, mp_actionRenderTileInfo(nullptr)
	, mp_actionClear(nullptr)
	, mp_actionProperties(nullptr)
	, m_FillMode(FILL_MODE::Solid)
	, m_ViewID(View::Top)
	, mb_View(true)
	, mp_ProfileWindow(nullptr)
	, m_FrameBuffer(-1)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setMinimumSize(50, 50);
	setFocusPolicy(Qt::FocusPolicy::ClickFocus);

	mp_Parent = static_cast<MainWindow*>(parent);
	assert(mp_Parent != nullptr);

	mf_Width = 1000.0;
	mf_Height = 1000.0;

	m_rctArea.x0 = -1;

	InitQuad(m_colorTEXT, 0.9F, 0.9F, 0.9F);

	InitQuad(m_colorAXIS_X, 1.0F, 0.0F, 0.0F);
	InitQuad(m_colorAXIS_Y, 0.0F, 1.0F, 0.0F);
	InitQuad(m_colorAXIS_Z, 0.0F, 0.0F, 1.0F);

	setMouseTracking(true);
	setAcceptDrops(true);

	// set up fonts

	m_Font.setFamily("Consolas");
	m_Font.setPointSize(10.0);
	m_Font.setItalic(false);
	
	CreateActions();

	if (model_type == (int)AeroMap::TerrainType::DTM)
		ms_FileName = tree.dtm_file;
	else
		ms_FileName = tree.dsm_file;
	SetCurrentFile(ms_FileName.c_str());
}

TerrainWindow::~TerrainWindow()
{
	// clean up opengl state, other gl windows will
	// re-inititialize it
	GLManager::Shutdown();

	//delete mp_ProfileWindow;

	delete mp_Terrain; mp_Terrain = nullptr;
	delete mp_actionViewTop; mp_actionViewTop = nullptr;
	delete mp_actionViewFront; mp_actionViewFront = nullptr;
	delete mp_actionViewSide; mp_actionViewSide = nullptr;
	delete mp_actionView3D; mp_actionView3D = nullptr;
	delete mp_actionFillSolid; mp_actionFillSolid = nullptr;
	delete mp_actionFillWire; mp_actionFillWire = nullptr;
	delete mp_actionRenderAxes; mp_actionRenderAxes = nullptr;
	delete mp_actionRenderDim; mp_actionRenderDim = nullptr;
	delete mp_actionRenderLights; mp_actionRenderLights = nullptr;
	delete mp_actionRenderSkySphere; mp_actionRenderSkySphere = nullptr;
	delete mp_actionRenderSkyBox; mp_actionRenderSkyBox = nullptr;
	delete mp_actionRenderSkyNone; mp_actionRenderSkyNone = nullptr;
	delete mp_actionRenderTerrain; mp_actionRenderTerrain = nullptr;
	delete mp_actionRenderTileInfo; mp_actionRenderTileInfo = nullptr;
	delete mp_actionClear; mp_actionClear = nullptr;
	delete mp_actionProperties; mp_actionProperties = nullptr;
}

void TerrainWindow::paintGL()
{
	if (GLManager::IsInitialized() == false)
		return;

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	QPainter painter;

	if (!painter.begin(this))
		assert(false);

	painter.beginNativePainting();

	m_winSize.cx = width();
	m_winSize.cy = height();

	// setup viewport
	glViewport(0, 0, (GLint)m_winSize.cx, (GLint)m_winSize.cy);

	// maintain model proportions on window resize
	double aspectRatio = (double)m_winSize.cy / (double)m_winSize.cx;
	mf_Height = aspectRatio * mf_Width;

	// clear back buffer

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, m_FillMode == FILL_MODE::Solid ? GL_FILL : GL_LINE);

	SetupMatrices();
	GLManager::PushMatrices(m_matModel, m_matView, m_matProjection);

	m_shaderPNT.Activate();

	if (mp_Terrain == nullptr)
	{
		if (ms_FileName.GetLength() > 0)
		{
			mp_Terrain = new TerrainGL(ms_FileName.c_str());
			OnViewTop();
			GLManager::CheckForOpenGLError(__FILE__, __LINE__);
		}
	}

	if (mp_Terrain)
	{
		mp_Terrain->Render(m_matModel, m_matView, m_matProjection, m_Camera, m_FrameBuffer, m_winSize,
						   NEAR_PLANE, FAR_PLANE);

		RenderDistance(&painter);
		RenderArea(&painter);

		if (mb_RenderDim)
			RenderDimensions();

		RenderPath(&painter);
		RenderSelectBox();
	}

	RenderText(&painter);

	painter.endNativePainting();
	painter.end();

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);
	GLManager::PopMatrices(m_matModel, m_matView, m_matProjection);
}

void TerrainWindow::RenderPath(QPainter* pPainter)
{
	// Render lines indicating path profile selection.
	//

	if (GetApp()->m_Tool.GetTool() != Tool::ToolType::Profile)
		return;
	if (mv_Path.size() < 1)
		return;

	GLManager::PushDepth(false);
	GLManager::PushCull(false);

	QPen penOld = pPainter->pen();

	QPainterPath path;
	path.moveTo(mv_Path[0].x, mv_Path[0].y);
	for (int i = 1; i < mv_Path.size(); ++i)
	{
		path.lineTo(mv_Path[i].x, mv_Path[i].y);
	}
	path.lineTo(m_ptLastMouse.x, m_ptLastMouse.y);

	pPainter->setPen(Qt::yellow);
	pPainter->drawPath(path);

	pPainter->setPen(penOld);

	GLManager::PopCull();
	GLManager::PopDepth();
}

void TerrainWindow::RenderDistance(QPainter* pPainter)
{
	// Render distance measurement.
	//

	if (m_ViewID == View::ThreeD)		// NA for 3D view
		return;
	if (mp_Terrain->GetDistancePointCount() < 1)
		return;

	GLManager::PushDepth(false);
	GLManager::PushCull(false);

	QPen penOld = pPainter->pen();

	int xp, yp;
	QPainterPath path;
	WorldToPixel(mp_Terrain->GetDistancePoint(0).x, mp_Terrain->GetDistancePoint(0).y, xp, yp);
	path.moveTo(static_cast<double>(xp), static_cast<double>(yp));

	char buf[32] = { 0 };
	pPainter->setPen(Qt::white);
	for (int i = 1; i < mp_Terrain->GetDistancePointCount(); ++i)
	{
		WorldToPixel(mp_Terrain->GetDistancePoint(i).x, mp_Terrain->GetDistancePoint(i).y, xp, yp);
		path.lineTo(xp, yp);

		sprintf(buf, "%0.1f", mp_Terrain->GetDistance(i));
		pPainter->drawText(xp, yp, buf);
	}

	// render preview for pending segment
	if (GetApp()->m_Tool.GetTool() == Tool::ToolType::Distance)
	{
		if ((mp_Terrain->GetDistancePointCount()) > 0 && (m_ptAnchor.x > -1))
		{
			path.lineTo(m_ptAnchor.x, m_ptAnchor.y);
		}
	}

	pPainter->setPen(Qt::yellow);
	pPainter->drawPath(path);

	pPainter->setPen(penOld);

	GLManager::PopCull();
	GLManager::PopDepth();
}

void TerrainWindow::RenderArea(QPainter* pPainter)
{
	// Render area measurement.
	//

	if (GetApp()->m_Tool.GetTool() != Tool::ToolType::Area)
		return;
	if (m_ViewID != View::Top)
		return;
	if (m_rctArea.x0 < 0)
		return;

	GLManager::PushDepth(false);
	GLManager::PushCull(false);

	QPen penOld = pPainter->pen();

	QPainterPath path;

	path.addRect(m_rctArea.x0, m_rctArea.y0, m_rctArea.Width(), m_rctArea.Height());

	char buf[32] = { 0 };
	pPainter->setPen(Qt::white);
	pPainter->setBrush(Qt::BrushStyle::ConicalGradientPattern);

	double wx0, wy0;
	double wx1, wy1;
	PixelToWorld(m_rctArea.x0, m_rctArea.y0, wx0, wy0);
	PixelToWorld(m_rctArea.x1, m_rctArea.y1, wx1, wy1);
	double area = ((wx1 - wx0) * 0.001) * ((wy1 - wy0)*0.001);
	sprintf(buf, "%0.1f sq km", fabs(area));
	pPainter->drawText(m_rctArea.x1 + 12, m_rctArea.y0 + 2, buf);

	pPainter->setPen(Qt::yellow);
	pPainter->drawPath(path);

	pPainter->setPen(penOld);

	GLManager::PopCull();
	GLManager::PopDepth();
}

void TerrainWindow::RenderDimensions()
{
	// Render model outer dimensions.

	if (mp_Terrain == nullptr)
		return;
	if (m_ViewID == View::ThreeD)		// NA for 3D view
		return;

	const int DIM_SIZE = 50.0;			// size of dim end lines
	const int DIM_SPACE = 10.0;			// space between model and dim end lines
	char buf[16] = { 0 };

	VEC3 color(0.9F, 0.9F, 0.9F);

	double sizeX = mp_Terrain->GetSizeX();		// outer dimension in x direction
	double sizeY = mp_Terrain->GetSizeY();
	int x0, y0, x1, y1;
	WorldToPixel(0.0, 0.0, x0, y0);
	WorldToPixel(sizeX, sizeY, x1, y1);

	GLManager::PushDepth(false);
	GLManager::PushCull(false);

	GLManager::PushMatrices(m_matModel, m_matView, m_matProjection);
	m_shaderSS.Activate();

	// set up screen space projection
	mat4 matProj = glm::ortho(0.0, (double)(m_winSize.cx - 1), (double)(m_winSize.cy - 1), 0.0);
	m_shaderSS.SetUniform("ProjectionMatrix", matProj);

	VertexBufferGL::VertexSSC vx;
	std::vector<VertexBufferGL::VertexSSC> vxList;

	// render left line

	int ymid = (y0 + y1) / 2;
	int x = x0 - DIM_SIZE * 0.5 - DIM_SPACE;

	vx.SetPos(x, y0);			vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x, ymid + 16);	vx.SetColor(0.0F);  vxList.push_back(vx);
	vx.SetPos(x, ymid - 16);	vx.SetColor(0.0F);  vxList.push_back(vx);
	vx.SetPos(x, y1);			vx.SetColor(color); vxList.push_back(vx);

	// top arrow
	vx.SetPos(x, y1);			vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x - 8, y1 + 16);	vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x + 8, y1 + 16);	vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x, y1);			vx.SetColor(color); vxList.push_back(vx);

	// bottom arrow
	vx.SetPos(x, y0);			vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x - 8, y0 - 16);	vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x + 8, y0 - 16);	vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x, y0);			vx.SetColor(color); vxList.push_back(vx);

	// bottom end line
	vx.SetPos(x0 - DIM_SPACE - DIM_SIZE, y0); vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x0 - DIM_SPACE, y0); vx.SetColor(color); vxList.push_back(vx);

	// top end line
	vx.SetPos(x0 - DIM_SPACE - DIM_SIZE, y1); vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x0 - DIM_SPACE, y1); vx.SetColor(color); vxList.push_back(vx);

	// render dimension

	QFontMetrics fm(m_Font);
	sprintf(buf, "%0.1f km", sizeY * 0.001);
	QRect rect = fm.boundingRect(buf);
	PushText(x - rect.width() / 2, ymid + rect.height() / 3, buf);

	// render bottom line

	int xmid = (x0 + x1) / 2;
	int y = y0 + DIM_SIZE * 0.5 + DIM_SPACE;

	vx.SetPos(x0, y);			vx.SetColor(color);	vxList.push_back(vx);
	vx.SetPos(xmid - 16, y);	vx.SetColor(0.0F);	vxList.push_back(vx);
	vx.SetPos(xmid + 16, y);	vx.SetColor(0.0F);	vxList.push_back(vx);
	vx.SetPos(x1, y);			vx.SetColor(color); vxList.push_back(vx);

	// left arrow
	vx.SetPos(x0, y);			vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x0 + 16, y - 8);	vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x0 + 16, y + 8);	vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x0, y);			vx.SetColor(color); vxList.push_back(vx);

	// right arrow
	vx.SetPos(x1, y);			vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x1 - 16, y - 8);	vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x1 - 16, y + 8);	vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x1, y);			vx.SetColor(color); vxList.push_back(vx);

	// left end line
	vx.SetPos(x0, y0 + DIM_SPACE + DIM_SIZE); vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x0, y0 + DIM_SPACE);            vx.SetColor(color); vxList.push_back(vx);

	// right end line
	vx.SetPos(x1, y0 + DIM_SPACE + DIM_SIZE); vx.SetColor(color); vxList.push_back(vx);
	vx.SetPos(x1, y0 + DIM_SPACE); vx.SetColor(color); vxList.push_back(vx);

	// render dimension

	sprintf(buf, "%0.1f km", sizeX * 0.001);
	rect = fm.boundingRect(buf);
	PushText(xmid - rect.width() / 2, y + rect.height() / 3, buf);
	
	VertexBufferGL vb;
	vb.CreateSSC(GL_LINES, vxList);
	vb.Render();

	GLManager::PopMatrices(m_matModel, m_matView, m_matProjection);

	GLManager::PopDepth();
	GLManager::PopCull();
}

void TerrainWindow::RenderSelectBox()
{
	//	render select box
	//

	if (m_ViewID == View::ThreeD)		// na for 3d view
		return;
	if (!mb_Selecting)					// don't render box unless we're actively selecting vertices
		return;

	GLManager::PushDepth(false);
	GLManager::PushCull(false);

	GLManager::PushMatrices(m_matModel, m_matView, m_matProjection);
	m_shaderSS.Activate();

	// set up screen space projection
	mat4 matProj = glm::ortho(0.0F, (float)(m_winSize.cx - 1), (float)(m_winSize.cy - 1), 0.0F);
	m_shaderSS.SetUniform("ProjectionMatrix", matProj);

	VertexBufferGL::VertexSSC vx;
	std::vector<VertexBufferGL::VertexSSC> vxList;

	vx.SetPos(m_rctSel.x0, m_rctSel.y0); vx.SetColor(1.0F, 0.5F, 1.0F); vxList.push_back(vx);
	vx.SetPos(m_rctSel.x1, m_rctSel.y0); vx.SetColor(1.0F, 0.5F, 1.0F); vxList.push_back(vx);
	vx.SetPos(m_rctSel.x1, m_rctSel.y1); vx.SetColor(1.0F, 0.5F, 1.0F); vxList.push_back(vx);
	vx.SetPos(m_rctSel.x0, m_rctSel.y1); vx.SetColor(1.0F, 0.5F, 1.0F); vxList.push_back(vx);

	VertexBufferGL vb;
	vb.CreateSSC(GL_LINE_LOOP, vxList);
	vb.Render();

	GLManager::PopMatrices(m_matModel, m_matView, m_matProjection);

	GLManager::PopDepth();
	GLManager::PopCull();
}

void TerrainWindow::RenderText(QPainter* pPainter)
{
	GLManager::PushDepth(false);

	// render any text items that were added to the list on this frame
	QPen penOld = pPainter->pen();
	pPainter->setPen(Qt::darkGray);
	pPainter->setFont(m_Font);

	int yoffset = 0;
	char buf[80] = { 0 };

	sprintf(buf, "Tool: %s", GetApp()->m_Tool.GetName());
	pPainter->drawText(2, yoffset += 12, buf);
	sprintf(buf, "View: %s", GetViewName());
	pPainter->drawText(2, yoffset += 12, buf);

	if (mb_DebugInfo == true)
	{
		// output debug info
		sprintf(buf, "Camera Pos: %0.1f, %0.1f, %0.1f", m_Camera.GetPos().x, m_Camera.GetPos().y, m_Camera.GetPos().z);
		pPainter->drawText(2, yoffset += 12, buf);
		sprintf(buf, "Camera Dir: %0.1f, %0.1f, %0.1f", m_Camera.GetDir().x, m_Camera.GetDir().y, m_Camera.GetDir().z);
		pPainter->drawText(2, yoffset += 12, buf);
		sprintf(buf, "Camera Up:  %0.1f, %0.1f, %0.1f", m_Camera.GetUp().x, m_Camera.GetUp().y, m_Camera.GetUp().z);
		pPainter->drawText(2, yoffset += 12, buf);

		////sprintf(buf, "Point Count:  %d", mp_Terrain->GetDistancePointCount());
		////pPainter->drawText(2, yoffset += 12, buf);
	}

	for (unsigned int i = 0; i < mv_Text.size(); ++i)
		pPainter->drawText(mv_Text[i].x, mv_Text[i].y, mv_Text[i].text);
	mv_Text.clear();

	pPainter->setPen(penOld);

	GLManager::PopDepth();
}

void TerrainWindow::PushText(int x, int y, const char* text)
{
	// Store all text to be rendered this frame to minimize
	// state switches.

	if (text == nullptr)
		return;

	TextType entry;
	memset(&entry, 0, sizeof(entry));
	entry.x = x;
	entry.y = y;
	if (strlen(text) > 0 && strlen(text) < sizeof(entry.text))
		strcpy(entry.text, text);
	mv_Text.push_back(entry);
}

double TerrainWindow::GetMinViewHeight()
{
	// Return the minimum height for the current 
	// camera that will not cut the frustum into the terrain

	// get terrain height at current camera location
	int row, col;
	mp_Terrain->XYToRowCol(m_Camera.GetPos().x, m_Camera.GetPos().y, row, col);
	double height = mp_Terrain->GetHeight(row, col);

	//TODO:
	//calc frustum xsection
	return height + 8.0;
}

void TerrainWindow::Excavate(RectType /*rectPix*/)
{
	// Excavate a rectangular patch.
	//
	// Inputs:
	//		rectPix = region to excavate, pixels

	// map the selection extents to terrain coordinates

	//int row0, col0, row1, col1;
	//PixelToTerrainRowCol(rectPix.x0, rectPix.y0, row0, col0);
	//PixelToTerrainRowCol(rectPix.x1, rectPix.y1, row1, col1);

	//if (row0 == row1)
	//	return;
	//if (col0 == col1)
	//	return;

	//RectType rect(col0, row0, col1, row1);
	//rect.Normalize();

	//long maxx = (long)mp_Terrain->GetColCount() - 1;
	//long maxy = (long)mp_Terrain->GetRowCount() - 1;

	//// region outside terrain
	//if (rect.x0 > maxx)
	//	return;
	//if (rect.y0 > maxy)
	//	return;
	//if (rect.x1 < 0)
	//	return;
	//if (rect.y1 < 0)
	//	return;

	//// clip at terrain boundaries
	//rect.x0 = rect.x0 < 0 ? 0 : rect.x0;
	//rect.y0 = rect.y0 < 0 ? 0 : rect.y0;
	//rect.x1 = rect.x1 > maxx ? maxx : rect.x1;
	//rect.y1 = rect.y1 > maxy ? maxy : rect.y1;

	//// excavate the patch

	//GetApp()->LogWrite(__FUNCTION__, "Excavating region ...");

	//double height = mp_Terrain->GetHeight((rect.y0 + rect.y1) / 2, (rect.x0 + rect.x1) / 2);
	//for (UInt16 r = rect.y0; r <= rect.y1; ++r)
	//{
	//	for (UInt16 c = rect.x0; c <= rect.x1; ++c)
	//	{
	//		mp_Terrain->SetHeight(r, c, height);
	//		mp_Terrain->SetFlags(r, c, static_cast<UInt8>(Terrain::Flags::DIRT));
	//	}
	//}

	//// update the texture

	//GetApp()->LogWrite(__FUNCTION__, "Updating texture ...");

	//QSettings settings(ORG_KEY, APP_KEY);
	//XString excavateFile = settings.value(TEX_EXCAVATE_KEY).toString();

	//ImageFile imgExc;		// excavation texture
	//if (imgExc.LoadFile(excavateFile.c_str()))
	//{
	//	int texScale = mp_Terrain->GetTextureScale();
	//	int imageHeight = texScale * mp_Terrain->GetRowCount();

	//	for (UInt16 r = rect.y0*texScale; r <= rect.y1*texScale; ++r)
	//	{
	//		for (UInt16 c = rect.x0*texScale; c <= rect.x1*texScale; ++c)
	//		{
	//			// get pixel from imgExc
	//			PixelType pix = imgExc.GetPixel(c % imgExc.GetWidth(), r % imgExc.GetHeight());

	//			// update the memory resident texture
	//			mp_Terrain->SetPixel(c, imageHeight - r - 1, pix);
	//		}
	//	}
	//}

	//mp_Terrain->Rebuild(rect);
	//update();

	//GetApp()->LogWrite(__FUNCTION__, "Excavation complete");
}

void TerrainWindow::Route(int startX, int startY, int endX, int endY)
{
	// Apply router tool.
	//
	// Applies router to line from (startX,startY) to (endX,endY)
	//
	// Inputs:
	//		(startX,startY) = line start point, pixel coordinates
	//		(endX,endY) = line end point, pixel coordinates

	int startRow, startCol;		// line endpoints, terrain row/col
	int endRow, endCol;
	PixelToTerrainRowCol(startX, startY, startRow, startCol);
	PixelToTerrainRowCol(endX, endY, endRow, endCol);

	// radius of tool, terrain data points
	int sizePts = static_cast<int>(GetApp()->m_Tool.GetMaskSize() / 2);

	std::vector<PointType> ptList;
	Bresenham(startCol, startRow, endCol, endRow, ptList);

	// for each pixel from last mouse pos to here
	for (unsigned int i = 0; i < ptList.size(); ++i)
	{
		double worldX, worldY;
		RectType rect;

		// router tool center point
		mp_Terrain->RowColToXY(ptList[i].y, ptList[i].x, worldX, worldY);

		rect.y0 = ptList[i].y - sizePts;
		rect.x0 = ptList[i].x - sizePts;
		rect.y1 = ptList[i].y + sizePts;
		rect.x1 = ptList[i].x + sizePts;

		// excavate the patch
		for (int r = rect.y0; r <= rect.y1; ++r)
		{
			for (int c = rect.x0; c <= rect.x1; ++c)
			{
				if ((r >= 0 && r < (int)mp_Terrain->GetRowCount()) && (c >= 0 && c < (int)mp_Terrain->GetColCount()))
				{
					int mr = r - rect.y0;
					int mc = c - rect.x0;
					double depth = GetApp()->m_Tool.GetDepth(mr, mc);
					if (depth > 0.0)
					{
						double height = mp_Terrain->GetHeight(r, c);
						mp_Terrain->SetHeight(r, c, height - depth);
					}
				}
			}
		}

		mp_Terrain->Rebuild(rect);
	}

	update();
}

void TerrainWindow::TexPaint(int startX, int startY, int endX, int endY)
{
	// Paint with texture.
	//
	// Applies paint to line from (startX,startY) to (endX,endY)
	//
	// Inputs:
	//		(startX,startY) = line start point, pixel coordinates
	//		(endX,endY) = line end point, pixel coordinates

	int startRow, startCol;		// line endpoints, terrain row/col
	int endRow, endCol;
	PixelToTerrainRowCol(startX, startY, startRow, startCol);
	PixelToTerrainRowCol(endX, endY, endRow, endCol);

	// radius of tool, terrain data points
	int sizePts = static_cast<int>(GetApp()->m_Tool.GetMaskSize() / 2);

	std::vector<PointType> ptList;
	if (endX > -1)
		Bresenham(startCol, startRow, endCol, endRow, ptList);
	else
		ptList.push_back(PointType(startX, startY));

	// for each pixel from last mouse pos to here
	for (unsigned int i = 0; i < ptList.size(); ++i)
	{
		double worldX, worldY;
		RectType rect;

		// tool center point
		mp_Terrain->RowColToXY(ptList[i].y, ptList[i].x, worldX, worldY);

		rect.y0 = ptList[i].y - sizePts;
		rect.x0 = ptList[i].x - sizePts;
		rect.y1 = ptList[i].y + sizePts;
		rect.x1 = ptList[i].x + sizePts;

		for (int r = rect.y0; r <= rect.y1; ++r)
		{
			for (int c = rect.x0; c <= rect.x1; ++c)
			{
				if ((r >= 0 && r < (int)mp_Terrain->GetRowCount()) && (c >= 0 && c < (int)mp_Terrain->GetColCount()))
				{
					int mr = r - rect.y0;
					int mc = c - rect.x0;
					PixelType pix = GetApp()->m_Tool.GetPixel(mr, mc);
					mp_Terrain->SetPixel(mc, mr, pix);
				}
			}
		}

		//mp_Terrain->Rebuild(rect);
	}

	update();
}

void TerrainWindow::ShowPathProfile()
{
	// Show path profile window.
	//
	// Inputs:
	//		mv_Path = list of screen coordinates defining path
	//

	//delete mp_ProfileWindow;

	if (mv_Path.size() >= 2)
	{
		std::vector<double> data;
		for (int seg = 0; seg < mv_Path.size() - 1; ++seg)
		{
			// get data along segment
			int point_count = Distance(mv_Path[seg].x, mv_Path[seg].y, mv_Path[seg + 1].x, mv_Path[seg + 1].y);
			double dx = mv_Path[seg + 1].x - mv_Path[seg].x;
			double dy = mv_Path[seg + 1].y - mv_Path[seg].y;
			// scale dx/dy to pixels
			dy = dy / (double)point_count;
			dx = dx / (double)point_count;

			double x = mv_Path[seg].x;
			double y = mv_Path[seg].y;
			for (int i = 0; i < point_count; ++i)
			{
				// sample point at (x,y)
				int pixel_x = (int)x;
				int pixel_y = (int)y;
				int row, col;
				PixelToTerrainRowCol(pixel_x, pixel_y, row, col);
				double elev = mp_Terrain->GetHeight(row, col);
				data.push_back(elev);

				// accum doubles, convert to int for buffer access
				x += dx;
				y += dy;
			}
		}

		// clear profile tool/setup
		mv_Path.clear();
		GetApp()->m_Tool.Clear();

		if (mp_ProfileWindow == nullptr)
			mp_ProfileWindow = new ProfileWindow(this);
		double dx = mp_Terrain->GetPitch();
		mp_ProfileWindow->SetData(dx, data);
		mp_ProfileWindow->showNormal();
	}
}

void TerrainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
	// This event handler can be reimplemented in a subclass to receive mouse 
	// double click events for the widget.
	//
	// The default implementation generates a normal mouse press event.
	//
	// Note: The widget will also receive mouse press and mouse release events in addition to 
	// the double click event. It is up to the developer to ensure that the application interprets 
	// these events correctly.

	Q_UNUSED(event);

	switch (GetApp()->m_Tool.GetTool()) {
	case Tool::ToolType::Profile:
		if (event->buttons() & Qt::LeftButton)
		{
			ShowPathProfile();
		}
		break;
	case Tool::ToolType::Distance:
		// basic way to stop measuring
		GetApp()->m_Tool.Clear();
		break;
	case Tool::ToolType::Area:
		GetApp()->m_Tool.Clear();
		break;
	default:
		// invert the current ortho view

		switch (m_ViewID) {
		case View::Front:
			mb_View = !mb_View;
			OnViewFront();
			break;
		case View::Side:
			mb_View = !mb_View;
			OnViewSide();
			break;
		}
	}
}

void TerrainWindow::mousePressEvent(QMouseEvent* event)
{
	// This event handler can be reimplemented in a subclass to receive mouse press events 
	// for the widget.
	// 
	// If you create new widgets in the mousePressEvent() the mouseReleaseEvent() may not 
	// end up where you expect, depending on the underlying window system (or X11 window manager), 
	// the widgets' location and maybe more.
	// 
	// The default implementation implements the closing of popup widgets when you click outside 
	// the window. For other widget types it does nothing.
	//

	if (m_ViewID == View::Top)
	{
		VEC2 pos;
		int row;
		int col;
		PixelToWorld(event->x(), event->y(), pos.x, pos.y);
		PixelToTerrainRowCol(event->x(), event->y(), row, col);

		switch (GetApp()->m_Tool.GetTool()) {
		case Tool::ToolType::Select:
		case Tool::ToolType::Excavate:
		case Tool::ToolType::ViewZoom:
		case Tool::ToolType::ViewPan:
			m_ptAnchor.x = event->x();
			m_ptAnchor.y = event->y();
			break;
		case Tool::ToolType::Distance:
			mp_Terrain->AddDistancePoint(pos);
			break;
		case Tool::ToolType::Area:
			// start/stop area rect
			if (m_rctArea.x0 < 0)
			{
				m_rctArea.x0 = event->x();
				m_rctArea.y0 = event->y();
				m_rctArea.x1 = event->x();
				m_rctArea.y1 = event->y();
			}
			else
			{
				m_rctArea.x0 = -1;
			}
			break;
		case Tool::ToolType::TexPaint:
			TexPaint(event->x(), event->y());
		case Tool::ToolType::Volume:
			break;
		case Tool::ToolType::Profile:
			if (event->buttons() & Qt::LeftButton)
			{
				// start/continue profile
				mv_Path.push_back(PointType(event->x(), event->y()));
			}
			break;
		default:   // no active tool
			break;
		}
	}

	update();

	m_ptLastMouse.x = event->x();  // last recorded mouse position
	m_ptLastMouse.y = event->y();
}

void TerrainWindow::mouseReleaseEvent(QMouseEvent* /* event */)
{
	// This event handler can be reimplemented in a subclass to receive 
	// mouse release events for the widget.
	//

	switch (GetApp()->m_Tool.GetTool()) {
	case Tool::ToolType::Select:   // finished selecting
		break;
	case Tool::ToolType::Excavate:
		if (m_ViewID != View::ThreeD)
		{
			if (mb_Selecting)
				Excavate(m_rctSel);
			mb_Selecting = false;
		}
		break;
	default:
		break;
	}

	update();
}

void TerrainWindow::mouseMoveEvent(QMouseEvent* event)
{
	// This event handler can be reimplemented in a subclass to receive mouse move 
	// events for the widget.
	// 
	// If mouse tracking is switched off, mouse move events only occur if a mouse button
	// is pressed while the mouse is being moved. If mouse tracking is switched on, mouse 
	// move events occur even if no mouse button is pressed.
	// 
	// QMouseEvent::pos() reports the position of the mouse cursor, relative to this widget. 
	// For press and release events, the position is usually the same as the position of the 
	// last mouse move event, but it might be different if the user's hand shakes. This is a 
	// feature of the underlying window system, not Qt.
	// 
	// If you want to show a tooltip immediately, while the mouse is moving (e.g., to get the
	// mouse coordinates with QMouseEvent::pos() and show them as a tooltip), you must first 
	// enable mouse tracking as described above. Then, to ensure that the tooltip is updated 
	// immediately, you must call QToolTip::showText() instead of setToolTip() in your 
	// implementation of mouseMoveEvent().
	//

	double mouseDX = (double)(event->x() - m_ptLastMouse.x);		// mouse offset relative to last call
	double mouseDY = (double)(event->y() - m_ptLastMouse.y);
	XString str;

	double worldX = 0.0;
	double worldY = 0.0;
	int row = 0;
	int col = 0;
	double heightM = 0.0;
	UInt8 flags = 0;
	bool onMap = false;

	// convert current mouse position to world units
	PixelToWorld(event->x(), event->y(), worldX, worldY);

	if (mp_Terrain)
	{
		onMap = mp_Terrain->XYToRowCol(worldX, worldY, row, col);
		if (onMap)
		{
			flags = mp_Terrain->GetFlags(row, col);

			heightM = mp_Terrain->GetHeightNormal(worldX, worldY);
		}
	}

	if (event->buttons() & Qt::LeftButton)		// left button is depressed
	{
		switch (m_ViewID) {
		case View::ThreeD:

			// 3d first person view from ground

			switch (GetApp()->m_Tool.GetTool()) {
			case Tool::ToolType::None:     // no tool selected
			{
				double delta = mouseDX * 0.05;	// scale it down
				if (event->modifiers() & Qt::ControlModifier)	// control key down
				{
					m_Camera.Rotate(delta * 0.25);
				}
				else
				{
					m_Camera.Pan(delta);		    // rotate view about z axis
				}
			}
			break;
			default:
				break;
			}   // end switch
			break;

		case View::Top:

			// 2d map view

			switch (GetApp()->m_Tool.GetTool()) {
			case Tool::ToolType::Select:
			case Tool::ToolType::Excavate:
				if (event->modifiers() & Qt::ControlModifier)	// control key down
				{
				}
				else
				{
					// set selection rectangle
					m_rctSel.x0 = m_ptAnchor.x;
					m_rctSel.y0 = m_ptAnchor.y;
					m_rctSel.x1 = event->x();
					m_rctSel.y1 = event->y();
					mb_Selecting = true;
				}
				break;
			case Tool::ToolType::TexPaint:
				//TexPaint(m_ptLastMouse.x, m_ptLastMouse.y, event->x(), event->y());
				break;
			case Tool::ToolType::Router:
				Route(m_ptLastMouse.x, m_ptLastMouse.y, event->x(), event->y());
				break;
			case Tool::ToolType::ViewZoom:
				{
					// adjust the view distance with vertical mouse movement

					// use camera's height for width of ortho view
					m_Camera.MoveZ(-mouseDY);
					UpdateOrthoScale();
				}
				break;
			case Tool::ToolType::ViewPan:
				{
					// to get it to pan the correct amount, need to convert mouse offsets to
					// world units

					double lastX = 0.0;
					double lastY = 0.0;

					// convert last mouse position to world units
					PixelToWorld(m_ptLastMouse.x, m_ptLastMouse.y, lastX, lastY);

					// difference them
					double dx = lastX - worldX;
					double dy = lastY - worldY;

					m_Camera.MoveX(dx);
					m_Camera.MoveY(dy);
				}
				break;
			case Tool::ToolType::None:     // no tool selected
				break;
			default:
				break;
			}

			break;

		case View::Front:
			switch (GetApp()->m_Tool.GetTool()) {
			case Tool::ToolType::ViewPan:
				{
					double lastX = 0.0;
					double lastY = 0.0;

					PixelToWorld(m_ptLastMouse.x, m_ptLastMouse.y, lastX, lastY);

					m_Camera.MoveX(mb_View ? lastX - worldX : worldX - lastX);
					m_Camera.MoveZ(lastY - worldY);
			}
				break;
			}
			break;
		case View::Side:
			switch (GetApp()->m_Tool.GetTool()) {
			case Tool::ToolType::ViewPan:
				{
					double lastX = 0.0;
					double lastY = 0.0;

					PixelToWorld(m_ptLastMouse.x, m_ptLastMouse.y, lastX, lastY);

					m_Camera.MoveY(mb_View ? lastX - worldX : worldX - lastX);
					m_Camera.MoveZ(lastY - worldY);
				}
				break;
			}
			break;
		}

		// tools that are independent of view
		switch (GetApp()->m_Tool.GetTool()) {
		case Tool::ToolType::Light:
			if (mp_Terrain)
			{
				double dx = mouseDX * 0.05;	// scale it down
				double dy = mouseDY * 0.05;

				mp_Terrain->GetLight().RotateAboutX(dy);
				mp_Terrain->GetLight().RotateAboutY(dx);
			}
		}

		update();
	}       // if left button
	else if (event->buttons() == 0)
	{
		// no button depressed
		switch (GetApp()->m_Tool.GetTool()) {
		case Tool::ToolType::None:     // no tool selected
			break;
		case Tool::ToolType::Distance:
			{
				// in process of measuring distance
				m_ptAnchor.x = event->x();
				m_ptAnchor.y = event->y();
			}
			break;
		case Tool::ToolType::Area:
			m_rctArea.x1 = event->x();
			m_rctArea.y1 = event->y();
			break;
		}
	}

	str.Clear();
	if (onMap)
	{
		XString strFlags;
		if (flags & static_cast<UInt8>(Terrain::Flags::WATER))
			strFlags += "WATER ";
		if (flags & static_cast<UInt8>(Terrain::Flags::DIRT))
			strFlags += "DIRT ";
		if (flags & static_cast<UInt8>(Terrain::Flags::SAND))
			strFlags += "SAND ";
		if (flags & static_cast<UInt8>(Terrain::Flags::ROCK))
			strFlags += "ROCK ";

		double lat = 0.0;
		double lon = 0.0;
		mp_Terrain->RowColToLL(row, col, lat, lon);
		str = XString::Format("Terrain Coordinates: (%0.1f, %0.1f)  Lat/Lon: (%0.5f,%0.5f) Row/Col: (%d, %d)  Elevation: %0.1fm  Flags: %s",
			worldX, worldY, lat, lon, row, col, heightM, strFlags.c_str());
	}
	mp_Parent->statusBar()->showMessage(str.c_str());

	update();

	m_ptLastMouse.x = event->x();
	m_ptLastMouse.y = event->y();
}

void TerrainWindow::wheelEvent(QWheelEvent* event)
{
	//	This event handler, for event event, can be reimplemented in a subclass to receive wheel events
	//	for the widget.
	//
	//	If you reimplement this handler, it is very important that you ignore() the event if you do not
	//	handle it, so that the widget's parent can interpret it.
	//
	//	The default implementation ignores the event.
	//

	if (event->angleDelta().y())
	{
		double delta = static_cast<double>(event->angleDelta().y()*-4);

		switch (GetApp()->m_Tool.GetTool()) {
		case Tool::ToolType::Light:
			if (mp_Terrain)
			{
				// wheel changes light distance
				VEC3 pos = mp_Terrain->GetLight().GetPos();
				VEC3 dir = pos.Unit();
				pos += dir * delta;

				// don't go below zero with distance, if negative
				// angle desired, use rotation
				if (pos.z > 0.0)
					mp_Terrain->GetLight().SetPos(pos);
			}
			break;
		default:
			{
			switch (m_ViewID) {
				case View::ThreeD:
					{
						double factor = fabs(delta);
						if (delta > 0)
							m_Camera.MoveForward(factor);
						else
							m_Camera.MoveBackward(factor);
						Update3DCameraHeight();
						break;
					}
				case View::Top:
					m_Camera.MoveZ(mb_View ? -delta : delta);
					UpdateOrthoScale();
					break;
				case View::Front:
					m_Camera.MoveY(mb_View ? -delta : delta);
					UpdateOrthoScale();
					break;
				case View::Side:
					m_Camera.MoveX(mb_View ? delta : -delta);
					UpdateOrthoScale();
					break;
				}
			}
			break;
		}
	}
	else
	{
		event->ignore();
	}

	update();
}

void TerrainWindow::keyPressEvent(QKeyEvent* event)
{
	// This event handler can be reimplemented in a subclass to receive key press 
	// events for the widget.
	// 
	// A widget must call setFocusPolicy() to accept focus initially and have focus 
	// in order to receive a key press event.
	// 
	// If you reimplement this handler, it is very important that you call the base 
	// class implementation if you do not act upon the key.
	// 
	// The default implementation closes popup widgets if the user presses Esc. 
	// Otherwise the event is ignored, so that the widget's parent can interpret it.
	// 
	// Note that QKeyEvent starts with isAccepted() == true, so you do not need to 
	// call QKeyEvent::accept() - just do not call the base class implementation if
	// you act upon the key.
	//

	// make move deltas relative to size of terrain
	double delta = mp_Terrain->GetRadius() * 0.01;

	switch (event->key()) {
	case Qt::Key_H:
		// "Home", reset view
		ResetView();
		break;
	case Qt::Key_D:
		mb_DebugInfo = !mb_DebugInfo;
		break;
	case Qt::Key_Up:
		if (event->modifiers() & Qt::CTRL)
			m_Camera.Tilt(0.01);
		else
			m_Camera.MoveForward(delta);
		break;
	case Qt::Key_Down:
		if (event->modifiers() & Qt::CTRL)
			m_Camera.Tilt(-0.01);
		else
			m_Camera.MoveBackward(delta);
		break;
	case Qt::Key_Left:
		if (event->modifiers() & Qt::CTRL)
			m_Camera.Pan(-0.1);
		else
			m_Camera.MoveLeft(delta);
		break;
	case Qt::Key_Right:
		if (event->modifiers() & Qt::CTRL)
			m_Camera.Pan(0.1);
		else
			m_Camera.MoveRight(delta);
		break;
	case Qt::Key_PageUp:
		m_Camera.MoveUp(delta);
		break;
	case Qt::Key_PageDown:
		m_Camera.MoveDown(delta);
		break;
	}

	update();

	__super::keyPressEvent(event);
}

void TerrainWindow::keyReleaseEvent(QKeyEvent* event)
{
	// This event handler can be reimplemented in a subclass to receive key release 
	// events for the widget.
	// 
	// A widget must accept focus initially and have focus in order to receive a key 
	// release event.
	// 
	// If you reimplement this handler, it is very important that you call the base 
	// class implementation if you do not act upon the key.
	// 
	// The default implementation ignores the event, so that the widget's parent can 
	// interpret it.
	// 
	// Note that QKeyEvent starts with isAccepted() == true, so you do not need to 
	// call QKeyEvent::accept() - just do not call the base class implementation if
	// you act upon the key.
	//

	__super::keyReleaseEvent(event);
}

void TerrainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	// This event handler is called when a drag is in progress and the mouse enters
	// this widget. The event is passed in the event parameter.
	//
	//	If the event is ignored, the widget won't receive any drag move events.
	


	//setText(tr("<drop content>"));
	//setBackgroundRole(QPalette::Highlight);

	event->acceptProposedAction();
	//emit changed(event->mimeData());
}

void TerrainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
	// This event handler is called when a drag is in progress and the mouse leaves
	// this widget.The event is passed in the event parameter.

	//clear();
	event->accept();
}

void TerrainWindow::dragMoveEvent(QDragMoveEvent *event)
{
	// This event handler is called if a drag is in progress, and when any of the
	// following conditions occur: the cursor enters this widget, the cursor moves
	// within this widget, or a modifier key is pressed on the keyboard while this 
	// widget has the focus.The event is passed in the event parameter.

	event->acceptProposedAction();
}

void TerrainWindow::dropEvent(QDropEvent* event)
{
	// This event handler is called when the drag is dropped on this widget.
	// The event is passed in the event parameter.
	//
	// The mimeData object can contain one of the following objects: 
	// an image, HTML text, plain text, or a list of URLs.

	//const QMimeData* mimeData = event->mimeData();
	event->acceptProposedAction();
}

void TerrainWindow::SetupMatrices()
{
	m_matModel = mat4(1.0f);
	m_matView = GLManager::MatViewFromCamera(m_Camera.GetPos(), m_Camera.GetDir(), m_Camera.GetUp());

	switch (m_ViewID) {
	case View::ThreeD:
	{
		double fov = 60.0;	// field of view, degrees
#ifdef GLM_FORCE_RADIANS
		fov = DegreeToRadian(fov);
#endif
		m_matProjection = glm::perspective(fov, (double)m_winSize.cx / (double)m_winSize.cy, NEAR_PLANE, FAR_PLANE);
		m_Camera.SetFrustum(fov, (double)m_winSize.cx / (double)m_winSize.cy, NEAR_PLANE, FAR_PLANE);
		break;
	}
	case View::Top:
	{
		// We only want the z depth to be large enough to show entire terrain. Making
		// it any deeper just reduces the z-buffer accuracy.
		double dz = 0.0;
		if (mp_Terrain)
			dz = mp_Terrain->GetMaxElev() - mp_Terrain->GetMinElev();
		double farPlane = m_Camera.GetPos().z + dz;
		m_matProjection = glm::ortho(-mf_Width*0.5, mf_Width*0.5, -mf_Height*0.5, mf_Height*0.5, NEAR_PLANE, farPlane);
		break;
	}
	case View::Front:
	case View::Side:
	{
		double farPlane = m_Camera.GetPos().Magnitude() + mp_Terrain->GetRadius();
		m_matProjection = glm::ortho(-mf_Width*0.5, mf_Width*0.5, -mf_Height*0.5, mf_Height*0.5, NEAR_PLANE, farPlane);
		break;
	}
	default:
		break;
	}

	mat4 mv = m_matView * m_matModel;

	m_shaderPNT.SetUniform("ModelViewMatrix", mv);
	m_shaderPNT.SetUniform("NormalMatrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	m_shaderPNT.SetUniform("MVP", m_matProjection * mv);

	m_shaderPNC.SetUniform("ModelViewMatrix", mv);
	m_shaderPNC.SetUniform("NormalMatrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	m_shaderPNC.SetUniform("MVP", m_matProjection * mv);

	m_shaderPC.SetUniform("MVP", m_matProjection * mv);
	m_shaderPCT.SetUniform("MVP", m_matProjection * mv);
}

void TerrainWindow::Update3DCameraHeight()
{
	// Keep the 3D camera on the surface without cutting
	// into terrain.
	//

	double height = 0.0;

	// if camera over terrain (else just use 0)
	if ((m_Camera.GetPos().x > 0.0 && m_Camera.GetPos().x < mp_Terrain->GetSizeX())
		&& (m_Camera.GetPos().y > 0.0 && m_Camera.GetPos().y < mp_Terrain->GetSizeY()))
	{
		height = mp_Terrain->GetHeightNormal(m_Camera.GetPos().x, m_Camera.GetPos().y);
	}

	m_Camera.SetZ(height + fabs(height) * 0.25);
}

void TerrainWindow::UpdateOrthoScale()
{
	// Update the dimensions of the ortho view
	// based on camera distance.
	//

	double aspectRatio = static_cast<double>(m_winSize.cy) / static_cast<double>(m_winSize.cx);
	mf_Width = Distance(VEC3(0, 0, 0), m_Camera.GetPos());
	mf_Height = aspectRatio * mf_Width;
}

bool TerrainWindow::PixelToTerrainRowCol(int x, int y, int& dbRow, int& dbCol)
{
	// Map window x/y values to terrain row/col.
	//
	// Inputs:
	//		x, y = 2d pixel values
	//
	// Outputs:
	//		row, col = terrain row, col
	//		return   = true if (x,y) on map, else row/col na
	//

	assert(m_ViewID != View::ThreeD);

	double worldX = 0.0;
	double worldY = 0.0;

	PixelToWorld(x, y, worldX, worldY);

	mp_Terrain->XYToRowCol(worldX, worldY, dbRow, dbCol);

	// off the map
	if (worldX < 0.0 || worldX > mp_Terrain->GetSizeX())
		return false;
	if (worldY < 0.0 || worldY > mp_Terrain->GetSizeY())
		return false;

	return true;
}

void TerrainWindow::PixelToWorld(int xp, int yp, double& worldX, double& worldY)
{
	// Map window x/y values to world xy values for current ortho view.
	//
	// Inputs:
	//		xp, yp = 2d pixel values
	//
	// Outputs:
	//		worldY, worldY = 2d world values
	//

	// calc pixel offset into window as factor from 0.0-1.0

	double dx = ((double)xp / (double)m_winSize.cx);			// +x == right
	double dy = 1.0 - ((double)yp / (double)m_winSize.cy);		// +y == up

	double minx = m_Camera.GetPos().x - mf_Width * 0.5;
	double miny = m_Camera.GetPos().y - mf_Height * 0.5;

	worldX = minx + mf_Width * dx;
	worldY = miny + mf_Height * dy;
}

bool TerrainWindow::WorldToPixel(double worldX, double worldY, int& xp, int& yp)
{
	// Map terrain (x,y) position to screen coordinates
	//
	// Inputs:
	//		worldY, worldY = 2d world values
	//
	// Outputs:
	//		xp, yp = 2d pixel values
	//		return = true if projected coordinates 
	//				 within window boundaries
	//

	return GLManager::Project(VEC3(worldX, worldY, 0), xp, yp);
}

const char* TerrainWindow::GetViewName()
{
	// return the name of the active view
	//

	const char* pViewName = "";

    switch (m_ViewID) {
	case View::Top:		pViewName = "Top";	 break;
	case View::Front:	pViewName = "Front"; break;
	case View::Side:	pViewName = "Side";  break;
	case View::ThreeD:  pViewName = "3D";    break;
	default: break;
    }

    return pViewName;
}

AXIS TerrainWindow::ViewToAxis(View view)
{
	// map orthogonal view ids to corresponding axes
	//

	AXIS nAxis = AXIS::None;

	switch (view)
	{
	case View::Top:		nAxis = AXIS::Z;   	break;
	case View::Front:	nAxis = AXIS::Y;   	break;
	case View::Side:	nAxis = AXIS::X;   	break;
	case View::ThreeD:	nAxis = AXIS::None;	break;		// does not map
	default: 			nAxis = AXIS::None;	break;
	}

	return nAxis;
}

bool TerrainWindow::Save()
{
	//if (mp_Terrain)
	//{
	//	GetApp()->LogWrite(__FUNCTION__, "Saving terrain...");
	//	mp_Terrain->Save();
	//	SetCurrentFile(mp_Terrain->GetMapFolder());
	//	GetApp()->LogWrite(__FUNCTION__, "Terrain saved");
	//}
	return true;
}

QString TerrainWindow::UserFriendlyCurrentFile()
{
	return StrippedName(ms_FileName.c_str());
}

void TerrainWindow::closeEvent(QCloseEvent* event)
{
	if (ConfirmClose())
	{
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void TerrainWindow::contextMenuEvent(QContextMenuEvent* event)
{
	// check selected items

	mp_actionViewTop->setChecked(m_ViewID == View::Top);
	mp_actionViewFront->setChecked(m_ViewID == View::Front);
	mp_actionViewSide->setChecked(m_ViewID == View::Side);
	mp_actionView3D->setChecked(m_ViewID == View::ThreeD);

	mp_actionFillSolid->setChecked(m_FillMode == FILL_MODE::Solid);
	mp_actionFillWire->setChecked(m_FillMode == FILL_MODE::Wire);

	mp_actionRenderAxes->setChecked(mp_Terrain->GetRenderAxes());
	mp_actionRenderDim->setChecked(mb_RenderDim);
	mp_actionRenderLights->setChecked(mp_Terrain->GetRenderLights());
	mp_actionRenderSkySphere->setChecked(mp_Terrain->GetRenderSky() == SkyBox::Mode::Sphere);
	mp_actionRenderSkyBox->setChecked(mp_Terrain->GetRenderSky() == SkyBox::Mode::Box);
	mp_actionRenderSkyNone->setChecked(mp_Terrain->GetRenderSky() == SkyBox::Mode::None);
	mp_actionRenderTerrain->setChecked(mp_Terrain->GetRenderTerrain());
	mp_actionRenderTileInfo->setChecked(mp_Terrain->GetRenderTileInfo());

	QMenu menu(this);
	menu.addAction(mp_actionViewTop);
	menu.addAction(mp_actionViewFront);
	menu.addAction(mp_actionViewSide);
	menu.addAction(mp_actionView3D);
	menu.addSeparator();
	menu.addAction(mp_actionFillSolid);
	menu.addAction(mp_actionFillWire);
	menu.addSeparator();
	menu.addAction(mp_actionRenderAxes);
	menu.addAction(mp_actionRenderDim);
	menu.addAction(mp_actionRenderLights);
	QMenu* pMenuSky = menu.addMenu("Render Sky");
	pMenuSky->addAction(mp_actionRenderSkySphere);
	pMenuSky->addAction(mp_actionRenderSkyBox);
	pMenuSky->addAction(mp_actionRenderSkyNone);
	menu.addAction(mp_actionRenderTerrain);
	menu.addAction(mp_actionRenderTileInfo);
	menu.addSeparator();
	menu.addAction(mp_actionClear);
	menu.addAction(mp_actionProperties);
	menu.exec(event->globalPos());
}

bool TerrainWindow::ConfirmClose()
{
	// Return true if ok close window.
	// Will save here if necessary.
	//

	bool close = true;
	//if (mp_Terrain->IsDirty())
	//{
	//	QMessageBox::StandardButton ret;
	//	ret = QMessageBox::warning(this, tr("MapEdit"),
	//		tr("'%1' has been modified.\n"
	//			"Do you want to save your changes?")
	//		.arg(mp_Terrain->GetTerrainFileName()),
	//		QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	//	switch (ret) {
	//	case QMessageBox::Save:
	//		Save();
	//		close = true;
	//		break;
	//	case QMessageBox::Cancel:
	//		close = false;
	//		break;
	//	case QMessageBox::Discard:
	//		close = true;
	//		break;
	//	}
	//}
	return close;
}

void TerrainWindow::SetCurrentFile(const QString& fileName)
{
	Q_UNUSED(fileName);

	//ms_TerrainFile = QFileInfo(fileName).canonicalFilePath();
	setWindowModified(false);
	setWindowTitle(GetCurrentFile() + "[*]");
}

QString TerrainWindow::GetCurrentFile()
{
	return ms_FileName.c_str();
}

QString TerrainWindow::StrippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void TerrainWindow::Contour()
{
	// Pass-through to terrain contouring algorithm.
	//

	if (mp_Terrain)
	{
		ContourDlg dlg(this);
		double minHeight = mp_Terrain->GetMinElev();
		double maxHeight = mp_Terrain->GetMaxElev();
		dlg.SetElevationRange(minHeight, maxHeight);

		if (dlg.exec() == QDialog::Accepted)
		{
			GetApp()->LogWrite(__FUNCTION__, "Creating contour lines ...");

			double z0 = dlg.GetStart();
			double dz = dlg.GetInterval();
			PixelType pixColor = dlg.GetContourColor();
			XString strScale = dlg.GetContourScale();

			// build vector of contour elevations
			std::vector<double> z;
			double height = z0;
			while (height < maxHeight)
			{
				z.push_back(height);
				height += dz;
			}

			mp_Terrain->Contour(z, pixColor, strScale);

			GetApp()->LogWrite(__FUNCTION__, "Contour complete");
		}
	}
	update();
}

void TerrainWindow::SmoothElevation()
{
	// Apply smoothing to entire terrain.
	//

	GetApp()->LogWrite(__FUNCTION__, "Applying smoothing ...");

	// create copy as smoothing source

	std::vector<double> ar;
	std::vector<std::vector<double>> a;

	for (int row = 0; row < mp_Terrain->GetRowCount(); ++row)
	{
		ar.clear();
		for (int col = 0; col < mp_Terrain->GetColCount(); ++col)
		{
			double elev = mp_Terrain->GetHeight(row, col);
			ar.push_back(elev);
		}
		a.push_back(ar);
	}

	// apply smoothing

	for (int row = 1; row < mp_Terrain->GetRowCount() - 1; ++row)
	{
		for (int col = 1; col < mp_Terrain->GetColCount() - 1; ++col)
		{
			double elev = a[row - 1][col - 1] + a[row - 1][col] + a[row - 1][col + 1]
						+ a[row][col - 1]     + a[row][col]     + a[row][col + 1]
						+ a[row + 1][col - 1] + a[row + 1][col] + a[row + 1][col + 1];
			elev /= 9.0;

			mp_Terrain->SetHeight(row, col, elev);
		}
	}

	mp_Terrain->Rebuild();
	update();

	GetApp()->LogWrite(__FUNCTION__, "Smoothing complete");
}

void TerrainWindow::ChangeElevation(double delta, double scaleFactor, bool isDelta)
{
	// Globally transform terrain elevation.
	//
	// Inputs:
	//		delta = fixed delta to apply to all terrain points
	//		scaleFactor = scale factor to multiply by all terrain points
	//		isDelta = use 'delta', else use 'scaleFactor'
	//

	GetApp()->LogWrite(__FUNCTION__, "Applying elevation ...");

	for (int row = 0; row < mp_Terrain->GetRowCount(); ++row)
	{
		for (int col = 0; col < mp_Terrain->GetColCount(); ++col)
		{
			double elev = mp_Terrain->GetHeight(row, col);
			if (isDelta)
				elev += delta;
			else
				elev *= scaleFactor;
			mp_Terrain->SetHeight(row, col, elev);
		}
	}

	mp_Terrain->Rebuild();
	update();

	GetApp()->LogWrite(__FUNCTION__, "Elevation complete");
}

void TerrainWindow::SetColorScale(XString scale)
{
	if (mp_Terrain)
	{
		if (scale == ORTHO_KEY)
		{
			mp_Terrain->SetTexture(ORTHO_KEY);
		}
		else
		{
			mp_Terrain->SetColorScale(scale);
		}
	}
	update();
}

void TerrainWindow::ResetView()
{
	// Reset to base map view, centered.

	OnViewTop();
}

void TerrainWindow::CreateActions()
{
	mp_actionViewTop = new QAction(QIcon(""), tr("Top View"), this);
	mp_actionViewTop->setStatusTip(tr("Orthographic top view"));
	mp_actionViewTop->setCheckable(true);
	connect(mp_actionViewTop, SIGNAL(triggered()), this, SLOT(OnViewTop()));

	mp_actionViewFront = new QAction(QIcon(""), tr("Front View"), this);
	mp_actionViewFront->setStatusTip(tr("Orthographic front view"));
	mp_actionViewFront->setCheckable(true);
	connect(mp_actionViewFront, SIGNAL(triggered()), this, SLOT(OnViewFront()));

	mp_actionViewSide = new QAction(QIcon(""), tr("Side View"), this);
	mp_actionViewSide->setStatusTip(tr("Orthographic side view"));
	mp_actionViewSide->setCheckable(true);
	connect(mp_actionViewSide, SIGNAL(triggered()), this, SLOT(OnViewSide()));

	mp_actionView3D = new QAction(QIcon(""), tr("3D View"), this);
	mp_actionView3D->setStatusTip(tr("3D perspective view"));
	mp_actionView3D->setCheckable(true);
	connect(mp_actionView3D, SIGNAL(triggered()), this, SLOT(OnView3D()));

	mp_actionFillSolid = new QAction(QIcon(""), tr("Solid"), this);
	mp_actionFillSolid->setStatusTip(tr("Render model as a solid"));
	mp_actionFillSolid->setCheckable(true);
	connect(mp_actionFillSolid, SIGNAL(triggered()), this, SLOT(OnFillSolid()));

	mp_actionFillWire = new QAction(QIcon(""), tr("Wire"), this);
	mp_actionFillWire->setStatusTip(tr("Render model as a wire frame"));
	mp_actionFillWire->setCheckable(true);
	connect(mp_actionFillWire, SIGNAL(triggered()), this, SLOT(OnFillWire()));

	mp_actionRenderAxes = new QAction(QIcon(""), tr("Render Axes"), this);
	mp_actionRenderAxes->setStatusTip(tr("Render Cartesian axes"));
	mp_actionRenderAxes->setCheckable(true);
	connect(mp_actionRenderAxes, SIGNAL(triggered()), this, SLOT(OnRenderAxes()));

	mp_actionRenderDim = new QAction(QIcon(""), tr("Render Dimensions"), this);
	mp_actionRenderDim->setStatusTip(tr("Render outer dimensions"));
	mp_actionRenderDim->setCheckable(true);
	connect(mp_actionRenderDim, SIGNAL(triggered()), this, SLOT(OnRenderDim()));

	mp_actionRenderLights = new QAction(QIcon(""), tr("Render Lights"), this);
	mp_actionRenderLights->setStatusTip(tr("Render light positions"));
	mp_actionRenderLights->setCheckable(true);
	connect(mp_actionRenderLights, SIGNAL(triggered()), this, SLOT(OnRenderLights()));

	mp_actionRenderSkySphere = new QAction(QIcon(""), tr("Sphere"), this);
	mp_actionRenderSkySphere->setStatusTip(tr("Render sky as a sphere"));
	mp_actionRenderSkySphere->setCheckable(true);
	connect(mp_actionRenderSkySphere, SIGNAL(triggered()), this, SLOT(OnRenderSkySphere()));

	mp_actionRenderSkyBox = new QAction(QIcon(""), tr("Sky Box"), this);
	mp_actionRenderSkyBox->setStatusTip(tr("Render skybox"));
	mp_actionRenderSkyBox->setCheckable(true);
	connect(mp_actionRenderSkyBox, SIGNAL(triggered()), this, SLOT(OnRenderSkyBox()));

	mp_actionRenderSkyNone = new QAction(QIcon(""), tr("None"), this);
	mp_actionRenderSkyNone->setStatusTip(tr("Do not render sky"));
	mp_actionRenderSkyNone->setCheckable(true);
	connect(mp_actionRenderSkyNone, SIGNAL(triggered()), this, SLOT(OnRenderSkyNone()));

	mp_actionRenderTerrain = new QAction(QIcon(""), tr("Render Terrain"), this);
	mp_actionRenderTerrain->setStatusTip(tr("Render terrain model"));
	mp_actionRenderTerrain->setCheckable(true);
	connect(mp_actionRenderTerrain, SIGNAL(triggered()), this, SLOT(OnRenderTerrain()));

	mp_actionRenderTileInfo = new QAction(QIcon(""), tr("Render Tile Info"), this);
	mp_actionRenderTileInfo->setStatusTip(tr("Render terrain tile outlines"));
	mp_actionRenderTileInfo->setCheckable(true);
	connect(mp_actionRenderTileInfo, SIGNAL(triggered()), this, SLOT(OnRenderTileInfo()));

	mp_actionClear = new QAction(QIcon(""), tr("Clear"), this);
	mp_actionClear->setStatusTip(tr("Clear edit artifacts"));
	connect(mp_actionClear, SIGNAL(triggered()), this, SLOT(OnClear()));

	mp_actionProperties = new QAction(QIcon(""), tr("Properties"), this);
	mp_actionProperties->setStatusTip(tr("Display terrain model properties"));
	connect(mp_actionProperties, SIGNAL(triggered()), this, SLOT(OnProperties()));
}

void TerrainWindow::initializeGL()
{
	GLManager::CheckForOpenGLError(__FILE__, __LINE__);
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if( GLEW_OK != err )
	{
		Logger::Write( __FUNCTION__, "Error initializing GLEW: %s", glewGetErrorString( err ) );
	}
	else
	{
		Logger::Write( __FUNCTION__, "Using GLEW: %s", glewGetString( GLEW_VERSION ) );
	}
	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	GLManager::Initialize();

	// log properties of gl / driver version

	Logger::Write(__FUNCTION__, "GLU_VERSION: %s", gluGetString(GLU_VERSION));
	Logger::Write(__FUNCTION__, "GLU_EXTENSIONS: %s", gluGetString(GLU_EXTENSIONS));

	GLint nMax = 0;
	glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, &nMax);          Logger::Write(__FUNCTION__, "GL_MAX_ATTRIB_STACK_DEPTH: %d", nMax);
	glGetIntegerv(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, &nMax);   Logger::Write(__FUNCTION__, "GL_MAX_CLIENT_ATTRIB_STACK_DEPTH: %d", nMax);
	glGetIntegerv(GL_MAX_CLIP_PLANES, &nMax);                 Logger::Write(__FUNCTION__, "GL_MAX_CLIP_PLANES: %d", nMax);
	glGetIntegerv(GL_MAX_EVAL_ORDER, &nMax);                  Logger::Write(__FUNCTION__, "GL_MAX_EVAL_ORDER: %d", nMax);
	glGetIntegerv(GL_MAX_LIGHTS, &nMax);                      Logger::Write(__FUNCTION__, "GL_MAX_LIGHTS: %d", nMax);
	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &nMax);       Logger::Write(__FUNCTION__, "GL_MAX_MODELVIEW_STACK_DEPTH: %d", nMax);
	glGetIntegerv(GL_MAX_NAME_STACK_DEPTH, &nMax);            Logger::Write(__FUNCTION__, "GL_MAX_NAME_STACK_DEPTH: %d", nMax);
	glGetIntegerv(GL_MAX_PIXEL_MAP_TABLE, &nMax);             Logger::Write(__FUNCTION__, "GL_MAX_PIXEL_MAP_TABLE: %d", nMax);
	glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &nMax);      Logger::Write(__FUNCTION__, "GL_MAX_PROJECTION_STACK_DEPTH: %d", nMax);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &nMax);                Logger::Write(__FUNCTION__, "GL_MAX_TEXTURE_SIZE: %d", nMax);
	glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &nMax);         Logger::Write(__FUNCTION__, "GL_MAX_TEXTURE_STACK_DEPTH: %d", nMax);

	GLint nViewPort[2];
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, (GLint*)nViewPort);
	Logger::Write(__FUNCTION__, "GL_MAX_VIEWPORT_DIMS: %dx%d", nViewPort[0], nViewPort[1]);

	//QGLFormat format = this->format();
	//Logger::Write(__FUNCTION__, "QGLFormat reports profile: ");
	//if (format.profile() == QGLFormat::CompatibilityProfile)
	//	Logger::Write(__FUNCTION__, "    compatibility.");
	//else if (format.profile() == QGLFormat::CoreProfile)
	//	Logger::Write(__FUNCTION__, "    core.");
	//else
	//	Logger::Write(__FUNCTION__, "    none.");

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* version = glGetString(GL_VERSION);
	const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	Logger::Write(__FUNCTION__, "GL Vendor    : %s", vendor);
	Logger::Write(__FUNCTION__, "GL Renderer  : %s", renderer);
	Logger::Write(__FUNCTION__, "GL Version   : %s", version);
	Logger::Write(__FUNCTION__, "GL Version   : %d.%d", major, minor);
	Logger::Write(__FUNCTION__, "GLSL Version : %s", glslVersion);

	//if( dumpExtensions ) {
	//GLint nExtensions;
	//glGetIntegerv( GL_NUM_EYTENSIONS, &nExtensions );
	//for( int i = 0; i < nExtensions; i++ ) {
	//Logger::Write( __FUNCTION__, "%s", glGetStringi(GL_EYTENSIONS, i) );
	//}
	//}

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	m_Camera.SetPos(0.0F, 50.0F, 0.0F);

	glClearColor(SCENE_BACK_COLOR.redF(), SCENE_BACK_COLOR.greenF(), SCENE_BACK_COLOR.blueF(), 1.0F);       	// Background
	glClearDepth(1.0);                             	// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);                       	// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);							// The Type Of Depth Testing To Do
	glEnable(GL_CULL_FACE);							// don't render back sides of polys
	glDisable(GL_BLEND);
	glShadeModel(GL_SMOOTH);                       	// Enable Smooth Shading

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	if (!InitializeShaders())
	{
		Logger::Write(__FUNCTION__, "InitializeShaders() failed.");
		exit(1);
	}

	// log opengl errors
	GLManager::CheckForOpenGLError(__FILE__, __LINE__);
}

void TerrainWindow::resizeGL(int w, int h)
{
	// do nothing with these, size related settings
	// handled in paint
	Q_UNUSED(w);
	Q_UNUSED(h);

	// now that i'm using multiple framebuffers with water rendering, 
	// i need to track "default" fb vs other fbs. in gl terms, "default"
	// is 0, but it looks like qt generates and binds a frame buffer

	// get that frame buffer id here and use it whenever "default"
	// framebuffer is needed

	// the working assumption here is that the fb id is set
	// by first resizeGL() call
	GLint drawFboId = 0, readFboId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);
	assert(drawFboId == readFboId);

	if (m_FrameBuffer < 0)
		m_FrameBuffer = drawFboId;
}

bool TerrainWindow::InitializeShaders()
{
	// Build shaders and add them to GLManager.
	//

	bool shaderStatus = true;

	// load and compile the PNT (position/normal/texture) shaders
	shaderStatus = m_shaderPNT.BuildShaderFromString("PNT", ShaderLib::GetVertexShaderPNT(), ShaderLib::GetFragmentShaderPNT());

	if (shaderStatus)
	{
		// load and compile the PNC (position/normal/color) shaders
		shaderStatus = m_shaderPNC.BuildShaderFromString("PNC", ShaderLib::GetVertexShaderPNC(), ShaderLib::GetFragmentShaderPNC());
	}

	if (shaderStatus)
	{
		// load and compile the PC (position/color) shaders
		shaderStatus = m_shaderPC.BuildShaderFromString("PC", ShaderLib::GetVertexShaderPC(), ShaderLib::GetFragmentShaderPC());
	}

	if (shaderStatus)
	{
		// load and compile the PCT (position/color/texcoord) shaders
		shaderStatus = m_shaderPCT.BuildShaderFromString("PCT", ShaderLib::GetVertexShaderPCT(), ShaderLib::GetFragmentShaderPCT());
	}

	if (shaderStatus)
	{
		// load and compile the SS (screen space) shaders
		shaderStatus = m_shaderSS.BuildShaderFromString("SS", ShaderLib::GetVertexShaderSS(), ShaderLib::GetFragmentShaderSS());
	}

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	// shaders are added to static GLManager so can be accessed
	// from other classes
	GLManager::AddShader("PNT", &m_shaderPNT);
	GLManager::AddShader("PNC", &m_shaderPNC);
	GLManager::AddShader("PC", &m_shaderPC);
	GLManager::AddShader("PCT", &m_shaderPCT);
	GLManager::AddShader("SS", &m_shaderSS);

	// here we set the default lighting and material attributes for all shaders
	m_shaderPNT.SetUniform("Light.Color", vec3(1.0f, 1.0f, 1.0f));
	m_shaderPNT.SetUniform("Light.Position", vec3(0.0f, 0.0f, 0.0f));
	m_shaderPNT.SetUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
	m_shaderPNT.SetUniform("Material.Kd", 0.9f, 0.9f, 0.9f);
	m_shaderPNT.SetUniform("Material.Ks", 0.1f, 0.1f, 0.1f);
	m_shaderPNT.SetUniform("Material.Shininess", 100.0f);

	m_shaderPNC.SetUniform("Light.Color", vec3(1.0f, 1.0f, 1.0f));
	m_shaderPNC.SetUniform("Light.Position", vec3(0.0f, 0.0f, 0.0f));
	m_shaderPNC.SetUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
	m_shaderPNC.SetUniform("Material.Kd", 0.9f, 0.9f, 0.9f);
	m_shaderPNC.SetUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
	m_shaderPNC.SetUniform("Material.Shininess", 100.0f);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	return shaderStatus;
}

// context menu handlers

void TerrainWindow::OnViewTop()
{
	// Activate orthographic "map" view.

	if (mp_Terrain == nullptr)
		return;

	m_ViewID = View::Top;

	double cx = mp_Terrain->GetColCount() * mp_Terrain->GetPitch();
	double cy = mp_Terrain->GetRowCount() * mp_Terrain->GetPitch();

	double distance = mp_Terrain->GetRadius()*1.5;
	if (mb_View)
	{
		m_Camera.SetPos(cx * 0.5, cy * 0.5, distance);
		m_Camera.SetDirUp(VEC3(0.0, 0.0, -1.0), VEC3(0.0, 1.0, 0.0));
	}
	else
	{
		m_Camera.SetPos(cx * 0.5, cy * 0.5, -distance);
		m_Camera.SetDirUp(VEC3(0.0, 0.0, 1.0), VEC3(0.0, 1.0, 0.0));
	}

	UpdateOrthoScale();

	// ortho views don't render sky
	mp_Terrain->SetRenderSky(SkyBox::Mode::None);

	update();
}

void TerrainWindow::OnViewFront()
{
	// Activate orthographic front view.

	if (mp_Terrain == nullptr)
		return;

	m_ViewID = View::Front;

	double cx = mp_Terrain->GetColCount() * mp_Terrain->GetPitch();

	double distance = mp_Terrain->GetRadius()*2.0;
	if (mb_View)
	{
		m_Camera.SetPos(cx * 0.5, -distance, 0.0);
		m_Camera.SetDirUp(VEC3(0.0, 1.0, 0.0), VEC3(0.0, 0.0, 1.0));
	}
	else
	{
		m_Camera.SetPos(cx * 0.5, distance, 0.0);
		m_Camera.SetDirUp(VEC3(0.0, -1.0, 0.0), VEC3(0.0, 0.0, 1.0));
	}

	UpdateOrthoScale();

	// ortho views don't render sky
	mp_Terrain->SetRenderSky(SkyBox::Mode::None);

	update();
}

void TerrainWindow::OnViewSide()
{
	// Activate orthographic side view.

	if (mp_Terrain == nullptr)
		return;

	m_ViewID = View::Side;

	double cy = mp_Terrain->GetRowCount() * mp_Terrain->GetPitch();

	double distance = mp_Terrain->GetRadius()*2.0;
	if (mb_View)
	{
		m_Camera.SetPos(distance, cy * 0.5, 0.0);
		m_Camera.SetDirUp(VEC3(-1.0, 0.0, 0.0), VEC3(0.0, 0.0, 1.0));
	}
	else
	{
		m_Camera.SetPos(-distance, cy * 0.5, 0.0);
		m_Camera.SetDirUp(VEC3(1.0, 0.0, 0.0), VEC3(0.0, 0.0, 1.0));
	}

	UpdateOrthoScale();

	// ortho views don't render sky
	mp_Terrain->SetRenderSky(SkyBox::Mode::None);

	update();
}

void TerrainWindow::OnView3D()
{
	// Switch to 3D view.

	if (mp_Terrain == nullptr)
		return;

	m_ViewID = View::ThreeD;

	GetApp()->m_Tool.Clear();

	// place camera at current mouse cursor position on surface looking north
	double x, y;
	PixelToWorld(m_ptLastMouse.x, m_ptLastMouse.y, x, y);
	m_Camera.SetPos(x, y, 0.0);
	m_Camera.SetDirUp(VEC3(0.0, 1.0, 0.0), VEC3(0.0, 0.0, 1.0));
	Update3DCameraHeight();

	// render sky in 3d view
	mp_Terrain->SetRenderSky(SkyBox::Mode::Box);

	update();
}

void TerrainWindow::OnFillSolid()
{
    m_FillMode = FILL_MODE::Solid;
    update();
}

void TerrainWindow::OnFillWire()
{
    m_FillMode = FILL_MODE::Wire;
    update();
}

void TerrainWindow::OnRenderAxes()
{
	if (mp_Terrain)
		mp_Terrain->SetRenderAxes(!mp_Terrain->GetRenderAxes());
	update();
}

void TerrainWindow::OnRenderDim()
{
	mb_RenderDim = !mb_RenderDim;
	update();
}

void TerrainWindow::OnRenderLights()
{
	if (mp_Terrain)
		mp_Terrain->SetRenderLights(!mp_Terrain->GetRenderLights());
	update();
}

void TerrainWindow::OnRenderSkySphere()
{
	if (mp_Terrain)
		mp_Terrain->SetRenderSky(SkyBox::Mode::Sphere);

	update();
}

void TerrainWindow::OnRenderSkyBox()
{
	if (mp_Terrain)
		mp_Terrain->SetRenderSky(SkyBox::Mode::Box);

	update();
}

void TerrainWindow::OnRenderSkyNone()
{
	if (mp_Terrain)
		mp_Terrain->SetRenderSky(SkyBox::Mode::None);

	update();
}

void TerrainWindow::OnRenderTerrain()
{
	// Render terrain model.

	if (mp_Terrain)
		mp_Terrain->SetRenderTerrain(!mp_Terrain->GetRenderTerrain());

	update();
}

void TerrainWindow::OnRenderTileInfo()
{
	// Render tile outlines.
	//

	if (mp_Terrain)
		mp_Terrain->SetRenderTileInfo(!mp_Terrain->GetRenderTileInfo());

	update();
}

void TerrainWindow::OnElevation()
{
	// Change the elevation of entire model.

	ElevationDlg dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		if (dlg.IsSmooth())
			SmoothElevation();
		else
			ChangeElevation(dlg.GetDelta(), dlg.GetScaleFactor(), dlg.IsDelta());
	}
}

void TerrainWindow::OnClear()
{
	// Clear editing artifacts.

	if (mp_Terrain)
	{
		mp_Terrain->ClearDistancePoints();
		mb_Selecting = false;
	}
	update();
}

void TerrainWindow::OnProperties()
{
	if (mp_Terrain)
	{
		XString str = mp_Terrain->GetMetaData();

		MetaDataDlg dlg(this, "Terrain Properties");
		dlg.SetMetaData(str);
		dlg.exec();
	}
}
