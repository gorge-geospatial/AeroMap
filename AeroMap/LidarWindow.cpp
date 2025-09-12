// LidarWindow.cpp
// Lidar rendering window.
//

#include <vector>

#include "ShaderLib.h"
#include "GLManager.h"
#include "MainWindow.h"
#include "LidarWindow.h"

#include <QStatusBar>

//TODO:
//make dynamic based on actual size
#define MAX_DEPTH 15000.0	// max viewing distance / backplane (making too deep decreases z-buffer accuracy)

LidarWindow::LidarWindow(QWidget* parent, int fileIndex)
	: QOpenGLWidget(parent)
	, m_LasFileIndex(fileIndex)
	, mp_LasFile(nullptr)
	, mp_VB(nullptr)
	, mp_ScaleColor(nullptr)
	, m_ScaleAttr(LidarAttr::Z)
	, mp_actionRenderAxes(nullptr)
	, mp_actionRenderDim(nullptr)
	, mb_RenderAxes(true)
	, mb_Selecting(false)
	, mb_DebugInfo(true)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setMinimumSize(640, 480);

	mp_Parent = static_cast<MainWindow*>(parent);
	assert(mp_Parent != nullptr);

	mp_LasFile = GetProject().GetLasFile(m_LasFileIndex);
	assert(mp_LasFile != nullptr);

	mf_Width = 1000.0;
	mf_Height = 1000.0;

	InitQuad(m_colorBACK, 0.0F, 0.0F, 0.0F);
	InitQuad(m_colorTEXT, 0.9F, 0.9F, 0.9F);
	InitQuad(m_colorDIM, 0.9F, 0.9F, 0.9F);

	InitQuad(m_colorAXIS_X, 1.0F, 0.0F, 0.0F);
	InitQuad(m_colorAXIS_Y, 0.0F, 1.0F, 0.0F);
	InitQuad(m_colorAXIS_Z, 0.0F, 0.0F, 1.0F);

	setMouseTracking(true);
	setAcceptDrops(true);
	grabKeyboard();

	// set up fonts

	m_Font.setFamily("Consolas");
	m_Font.setPointSize(10.0);
	m_Font.setItalic(false);
	
	CreateActions();
}

LidarWindow::~LidarWindow()
{
	delete mp_ScaleColor;

	DeleteVertexBuffer();

	// clean up opengl state, other gl windows will
	// re-inititialize it
	GLManager::Shutdown();
}

void LidarWindow::paintGL()
{
	if (GLManager::IsInitialized() == false)
		return;

	QPainter painter;

	if (!painter.begin(this))
		assert(false);

	painter.beginNativePainting();

	{
		m_winSize.cx = width();
		m_winSize.cy = height();

		// setup viewport, projection etc.:
		glViewport(0, 0, (GLint)m_winSize.cx, (GLint)m_winSize.cy);

		// maintain model proportions on window resize
		float aspectRatio = (float)m_winSize.cy / (float)m_winSize.cx;
		mf_Height = aspectRatio * mf_Width;
	}

	// clear back buffer

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	SetupMatrices();
	GLManager::PushMatrices(m_matModel, m_matView, m_matProjection);

	m_shaderPC.Activate();

	// no render source, create it
	if (mp_VB == nullptr)
	{
		if (mp_LasFile != nullptr)
		{
			CreateVertexBuffer();
		}
	}

	if (mp_VB)
		mp_VB->Render();

	if (mb_RenderAxes)
		RenderAxes();

	RenderSelectBox();
	RenderText(&painter);

	painter.endNativePainting();
	painter.end();

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);
	GLManager::PopMatrices(m_matModel, m_matView, m_matProjection);
}

void LidarWindow::CreateVertexBuffer()
{
	// Create vertex buffer used for rendering point cloud.
	//

	assert(mp_LasFile != nullptr);

	DeleteVertexBuffer();
	mp_VB = new VertexBufferGL();

	double z_range = mp_LasFile->GetMaxZ() - mp_LasFile->GetMinZ();
	std::vector<VertexBufferGL::VertexPC> pointList;
	for (UInt64 i = 0; i < mp_LasFile->GetPointCount(); ++i)
	{
		VertexBufferGL::VertexPC pt;
		VEC3 pos = mp_LasFile->GetScaledXYZ(i);
		pt.x = pos.x;
		pt.y = pos.y;
		pt.z = pos.z;

		// select point color based on attribute / color scale
		switch (m_ScaleAttr) {
			case LidarAttr::Z:
				if (mp_ScaleColor == nullptr)
				{
					// default to gray scale
					float sf = 0.5 + 0.5 * (pos.z - mp_LasFile->GetMinZ()) / z_range;
					pt.r = pt.g = pt.b = sf;
				}
				else
				{
					double sf = (pos.z - mp_LasFile->GetMinZ()) / z_range;
					PixelType pix = mp_ScaleColor->GetColorByHeight(sf);
					pt.r = pix.GetR();
					pt.g = pix.GetG();
					pt.b = pix.GetB();
				}
				break;
			case LidarAttr::Class:
				{
					//Classification codes: LAS format 1.1 - 1.4
					//
					//	0	Never classified
					//	1	Unassigned
					//	2	Ground
					//	3	Low Vegetation
					//	4	Medium Vegetation
					//	5	High Vegetation
					//	6	Building
					//	7	Low Point
					//	8	Reserved
					//	9	Water
					// 10	Rail
					// 11	Road Surface
					// 12	Reserved
					// 13	Wire - Guard (Shield)
					// 14	Wire - Conductor (Phase)
					// 15	Transmission Tower
					// 16	Wire-Structure Connector (Insulator)
					// 17	Bridge Deck
					// 18	High Noise
					// 19-63	Reserved
					// 64-255	User Definable
					int pt_class = mp_LasFile->GetPointClass(i);
					if (mp_ScaleColor == nullptr)
					{
						// assign hard-coded values to each of the 19 classes
						assert(false);
					}
					else
					{
						double sf = (double)pt_class / 20.0;
						PixelType pix = mp_ScaleColor->GetColorByHeight(sf);
						pt.r = pix.GetR();
						pt.g = pix.GetG();
						pt.b = pix.GetB();
					}
				}
				break;
			case LidarAttr::Return:
				{
					// point rec types 0-5 have 3 bit return #s (1-5), from 6 on,
					// they have 4 bit values (1-15)
					int ret_max = mp_LasFile->GetReturnMax();
					int pt_return = mp_LasFile->GetReturnNumber(i);
					if (mp_ScaleColor == nullptr)
					{
						assert(false);
					}
					else
					{
						double sf = (double)pt_return / (double)ret_max;
						PixelType pix = mp_ScaleColor->GetColorByHeight(sf);
						pt.r = pix.GetR();
						pt.g = pix.GetG();
						pt.b = pix.GetB();
					}
				}
				break;
		}

		pointList.push_back(pt);
	}
	mp_VB->CreatePC(GL_POINTS, pointList);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);
}

void LidarWindow::DeleteVertexBuffer()
{
	if (mp_VB)
	{
		mp_VB->Delete();
		delete mp_VB;
		mp_VB = nullptr;
	}
}

void LidarWindow::RenderAxes()
{
	// render cartesian axes for entire world

	float axisLength = 1000.0F;

	VEC3 center;
	if (mp_LasFile)
	{
		center.x = (mp_LasFile->GetMaxX() + mp_LasFile->GetMinX()) * 0.5;
		center.y = (mp_LasFile->GetMaxY() + mp_LasFile->GetMinY()) * 0.5;
		center.z = (mp_LasFile->GetMaxZ() + mp_LasFile->GetMinZ()) * 0.5;
	}

	VertexBufferGL::VertexPC vx;
	std::vector<VertexBufferGL::VertexPC> vxList;

	// render the XYZ axes according to application coordinate system, not OpenGL's
	vx.SetPos(center.x - axisLength, 0, 0); vx.SetColor(0, 0, 0); vxList.push_back(vx);
	vx.SetPos(center.x + axisLength, 0, 0); vx.SetColor(1, 0, 0); vxList.push_back(vx);

	vx.SetPos(0, center.y - axisLength, 0); vx.SetColor(0, 0, 0); vxList.push_back(vx);
	vx.SetPos(0, center.y + axisLength, 0); vx.SetColor(0, 1, 0); vxList.push_back(vx);

	vx.SetPos(0, 0, center.z - axisLength); vx.SetColor(0, 0, 0); vxList.push_back(vx);
	vx.SetPos(0, 0, center.z + axisLength); vx.SetColor(0, 0, 1); vxList.push_back(vx);

	m_shaderPC.Activate();

	VertexBufferGL vbAxes;
	vbAxes.CreatePC(GL_LINES, vxList);
	vbAxes.Render();
}

void LidarWindow::RenderDistance(QPainter* pPainter)
{
	// render distance measurement

	//if (m_ViewID == View::ThreeD)		// NA for 3D view
	//	return;
	//if (mp_Terrain->GetDistancePointCount() < 2)
	//	return;

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	//QPen penOld = pPainter->pen();

	//int xp, yp;
	//QPainterPath path;
	//WorldToPixel(mp_Terrain->GetDistancePoint(0)->x, mp_Terrain->GetDistancePoint(0)->y, xp, yp);
	//path.moveTo(static_cast<double>(xp), m_winSize.cy - static_cast<double>(yp));

	//char buf[32];
	pPainter->setPen(Qt::white);
	//for (int i = 1; i < mp_Terrain->GetDistancePointCount(); ++i)
	//{
	//	WorldToPixel(mp_Terrain->GetDistancePoint(i)->x, mp_Terrain->GetDistancePoint(i)->y, xp, yp);
	//	path.lineTo(xp, m_winSize.cy - yp);

	//	sprintf(buf, "%0.1f", mp_Terrain->GetDistance(i));
	//	pPainter->drawText(xp, m_winSize.cy - yp, buf);
	//}

	//pPainter->setPen(Qt::yellow);
	//pPainter->drawPath(path);

	//pPainter->setPen(penOld);
}

void LidarWindow::RenderSelectBox()
{
	//	render select box
	//

	if (!mb_Selecting)					// don't render box unless we're actively selecting vertices
		return;

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

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
}

void LidarWindow::RenderText(QPainter* pPainter)
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	GLManager::PushMatrices(m_matModel, m_matView, m_matProjection);
	m_shaderSS.Activate();

	// set up screen space projection
	mat4 matProj = glm::ortho(0.0F, (float)(m_winSize.cx - 1), (float)(m_winSize.cy - 1), 0.0F);
	m_shaderSS.SetUniform("ProjectionMatrix", matProj);

	// render any text items that were added to the list on this frame
	QPen penOld = pPainter->pen();
	pPainter->setPen(Qt::black);
	pPainter->setFont(m_Font);

	int yoffset = 0;
	char buf[80];
	sprintf(buf, "Lidar");
	pPainter->drawText(2, yoffset += 12, buf);
	sprintf(buf, "Tool: %s", GetApp()->m_Tool.GetName());
	pPainter->drawText(2, yoffset += 12, buf);

	if (mb_DebugInfo)
	{
		// output debug info
		sprintf(buf, "Camera Pos: %0.1f, %0.1f, %0.1f", m_Camera.GetPos().x, m_Camera.GetPos().y, m_Camera.GetPos().z);
		pPainter->drawText(2, yoffset += 12, buf);
		sprintf(buf, "Camera Dir: %0.1f, %0.1f, %0.1f", m_Camera.GetDir().x, m_Camera.GetDir().y, m_Camera.GetDir().z);
		pPainter->drawText(2, yoffset += 12, buf);
		sprintf(buf, "Camera Up:  %0.1f, %0.1f, %0.1f", m_Camera.GetUp().x, m_Camera.GetUp().y, m_Camera.GetUp().z);
		pPainter->drawText(2, yoffset += 12, buf);

		if (mp_LasFile)
		{
			sprintf(buf, "X Range: %0.1f - %0.1f", mp_LasFile->GetMinX(), mp_LasFile->GetMaxX());
			pPainter->drawText(2, yoffset += 12, buf);
			//sprintf(buf, "Y Range: %0.1f - %0.1f", min.y, max.y);
			//pPainter->drawText(2, yoffset += 12, buf);
			//sprintf(buf, "Z Range: %0.1f - %0.1f", min.z, max.z);
			//pPainter->drawText(2, yoffset += 12, buf);
			sprintf(buf, "Point Count:  %llu", mp_LasFile->GetPointCount());
			pPainter->drawText(2, yoffset += 12, buf);
		}
	}

	pPainter->setPen(penOld);

	GLManager::PopMatrices(m_matModel, m_matView, m_matProjection);
}

void LidarWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
	// This event handler can be reimplemented in a subclass to receive mouse 
	// double click events for the widget.
	// The default implementation generates a normal mouse press event.
	// Note: The widget will also receive mouse press and mouse release events in addition to 
	// the double click event. It is up to the developer to ensure that the application interprets 
	// these events correctly.

	Q_UNUSED(event);

	update();
}

void LidarWindow::mousePressEvent(QMouseEvent* event)
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

	switch (GetApp()->m_Tool.GetTool()) {
	case Tool::ToolType::Select:
	case Tool::ToolType::ViewZoom:
	case Tool::ToolType::ViewPan:
		m_ptAnchor.x = event->x();
		m_ptAnchor.y = event->y();
		break;
	case Tool::ToolType::Distance:
		{
			//VEC2 pos;
			//PixelToWorld(event->x(), event->y(), pos.x, pos.y);
			//mp_Terrain->AddDistancePoint(pos);
		}
		break;
	case Tool::ToolType::Area:
		break;
	case Tool::ToolType::Volume:
		break;
	default:   // no active tool
		break;
	}

	update();

	m_ptLastMouse.x = event->x();  // last recorded mouse position
	m_ptLastMouse.y = event->y();
}

void LidarWindow::mouseReleaseEvent(QMouseEvent* /* event */)
{
	// This event handler can be reimplemented in a subclass to receive 
	// mouse release events for the widget.
	//

	switch (GetApp()->m_Tool.GetTool()) {
	case Tool::ToolType::Select:   // finished selecting
		break;
	default:
		break;
	}

	update();
}

void LidarWindow::mouseMoveEvent(QMouseEvent* event)
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

	//double deltaX = (double)(event->x() - m_ptLastMouse.x);		// mouse offset relative to last call
	double deltaY = (double)(event->y() - m_ptLastMouse.y);
	XString str;

	// convert current mouse position to world units
	double curX = 0.0;
	double curY = 0.0;
	PixelToWorld(event->x(), event->y(), curX, curY);

	if (event->buttons() & Qt::LeftButton)		// left button is depressed
	{
		switch (GetApp()->m_Tool.GetTool()) {
		case Tool::ToolType::Select:
		case Tool::ToolType::ViewZoom:
			{
				// adjust the view distance with vertical mouse movement

				// use camera's height for width of ortho view
				m_Camera.MoveZ(-deltaY);
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
				double dx = lastX - curX;		// horizontal coordinate = x
				double dy = lastY - curY;		// vertical coordinate = z

				m_Camera.MoveX(dx);
				m_Camera.MoveY(dy);
			}
			break;
		case Tool::ToolType::None:     // no tool selected
			break;
		default:
			break;
		}

		update();
	}       // if left button

	str = XString::Format("Mouse: (%0.1f, %0.1f)", curX, curY);
	mp_Parent->statusBar()->showMessage(str.c_str());

	m_ptLastMouse.x = event->x();
	m_ptLastMouse.y = event->y();
}

void LidarWindow::wheelEvent(QWheelEvent* event)
{
	//	This event handler, for event event, can be reimplemented in a subclass to receive wheel events
	//	for the widget.
	//
	//	If you reimplement this handler, it is very important that you ignore() the event if you do not
	//	handle it, so that the widget's parent can interpret it.
	//
	//	The default implementation ignores the event.
	//

	int delta = event->angleDelta().y();
	if (delta)
	{
		// to start, wheel will just zoom, but may want to tie it to current tool
		// or mouse buttons, etc.

		// use camera's height for width of ortho view
		m_Camera.MoveZ((float)delta);
		UpdateOrthoScale();
		
		update();
	}
	else
	{
		event->ignore();
	}
}

void LidarWindow::keyPressEvent(QKeyEvent* event)
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

	switch (event->key()) {
	case Qt::Key_D:
		mb_DebugInfo = !mb_DebugInfo;
		break;
	case Qt::Key_S:
		m_Camera.MoveBackward(1.0);
		break;
	case Qt::Key_W:
		m_Camera.MoveForward(1.0);
		break;
	}

	update();

	__super::keyPressEvent(event);
}

void LidarWindow::keyReleaseEvent(QKeyEvent* event)
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

void LidarWindow::SetupMatrices()
{
	m_matModel = mat4(1.0f);

	// in all cases eypoint coincides with camera
	vec3 eye(m_Camera.GetPos().x, m_Camera.GetPos().y, m_Camera.GetPos().z);
	vec3 up(m_Camera.GetUp().x, m_Camera.GetUp().y, m_Camera.GetUp().z);
	vec3 lookAt(m_Camera.GetPos().x + m_Camera.GetDir().x, m_Camera.GetPos().y + m_Camera.GetDir().y, m_Camera.GetPos().z + m_Camera.GetDir().z);

	m_matView = glm::lookAt(eye, lookAt, up);
	m_matProjection = glm::ortho(-mf_Width*0.5, mf_Width*0.5, -mf_Height*0.5, mf_Height*0.5, 1.0, MAX_DEPTH);

	mat4 mv = m_matView * m_matModel;

	m_shaderPNT.SetUniform("ModelViewMatrix", mv);
	m_shaderPNT.SetUniform("NormalMatrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	m_shaderPNT.SetUniform("MVP", m_matProjection * mv);

	m_shaderPC.SetUniform("MVP", m_matProjection * mv);
}

void LidarWindow::UpdateOrthoScale()
{
	// update the dimensions of the ortho view
	// based on camera height
	//

	double aspectRatio = (double)m_winSize.cy / (double)m_winSize.cx;
	mf_Width = m_Camera.GetPos().z;
	mf_Height = aspectRatio * mf_Width;
}

void LidarWindow::PixelToWorld(int xp, int yp, double& worldX, double& worldY)
{
	// map window x/y values to world xy values for current ortho view
	//
	//	inputs:
	//		xp, yp = 2d pixel values
	//
	//	outputs:
	//		worldY, worldY = 2d world values
	//

	// calc pixel offset into window as factor from 0.0-1.0

	double dx = ((double)xp / (double)m_winSize.cx);						// +x == right
	double dy = ((double)(m_winSize.cy - yp) / (double)m_winSize.cy);		// +y == up

	// the "Y" center is camera.x
	double minx = m_Camera.GetPos().x - mf_Width * 0.5;
	double miny = m_Camera.GetPos().y - mf_Height * 0.5;

	worldX = minx + mf_Width * dx;
	worldY = miny + mf_Height * dy;
}

bool LidarWindow::WorldToPixel(double worldX, double worldY, int& xp, int& yp)
{
	// map terrain (x,y) position to screen coordinates
	//
	//	inputs:
	//		worldY, worldY = 2d world values
	//
	//	outputs:
	//		xp, yp = 2d pixel values
	//		return = true if projected coordinates 
	//				 within window boundaries
	//

	return GLManager::Project(VEC3(worldX, worldY, 0), xp, yp);
}

void LidarWindow::SetColorScale(LidarAttr scaleAttr, const char* scaleFile)
{
	// Set the active color scale.
	//
	// Inputs:
	//		scaleAttr = attribute to which color is applied
	//		scaleFile = color scale to use
	//

	assert(scaleFile != nullptr);

	if (QFile::exists(scaleFile) == false)
		return;

	delete mp_ScaleColor;
	mp_ScaleColor = new ScaleColor(scaleFile);
	m_ScaleAttr = scaleAttr;

	DeleteVertexBuffer();

	// save the selection
	//QSettings settings(ORG_KEY, APP_KEY);
	//settings.setValue(LAST_LIDAR_SCALE_ATTR_KEY, scaleAttr);
	//settings.setValue(LAST_LIDAR_SCALE_FILE_KEY, scaleFile);

	update();
}

void LidarWindow::SetBackColor(UInt8 R, UInt8 G, UInt8 B)
{
	m_colorBACK[0] = (float)R / 255.0F;
	m_colorBACK[1] = (float)G / 255.0F;
	m_colorBACK[2] = (float)B / 255.0F;

	glClearColor(m_colorBACK[0], m_colorBACK[1], m_colorBACK[2], 1.0);

	update();
}

void LidarWindow::GetBackColor(UInt8& R, UInt8& G, UInt8& B)
{
	R = (UInt8)(m_colorBACK[0] * 255.0);
	G = (UInt8)(m_colorBACK[1] * 255.0);
	B = (UInt8)(m_colorBACK[2] * 255.0);
}

bool LidarWindow::Save()
{
  //  if (mb_isUntitled)
  //  {
  //      return SaveAs();
  //  }
  //  else
  //  {
		//return SaveFile(ms_FileName);
  //  }
	return false;
}

bool LidarWindow::SaveAs()
{
//TODO:
//terrain saved while editing - should that change?
	//QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), ms_FileName);
	//if (fileName.isEmpty())
	//	return false;

	//return SaveFile(fileName);
	return false;
}

bool LidarWindow::SaveFile(const QString &fileName)
{
	//TODO:
	//terrain saved while editing - should that change?
	SetCurrentFile(fileName);
	return true;
}

LasFile* LidarWindow::GetLasFile()
{
	// Get reference to backing lasfile object.
	//

	return mp_LasFile;
}

void LidarWindow::closeEvent(QCloseEvent* event)
{
	if (maybeSave())
	{
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void LidarWindow::contextMenuEvent(QContextMenuEvent* event)
{
	// check selected items

	mp_actionRenderAxes->setChecked(mb_RenderAxes);
	mp_actionRenderDim->setChecked(mb_RenderDim);

	QMenu menu(this);
	menu.addAction(mp_actionRenderAxes);
	menu.addAction(mp_actionRenderDim);
	menu.exec(event->globalPos());
}

void LidarWindow::documentWasModified()
{
	//setWindowModified(document()->isModified());
}

bool LidarWindow::maybeSave()
{
//  if (document()->isModified())
//  {
//      QMessageBox::StandardButton ret;
//      ret = QMessageBox::warning(this, tr("AeroMap"),
//                   tr("'%1' has been modified.\n"
//                      "Do you want to save your changes?")
//                   .arg(userFriendlyCurrentFile()),
//                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//      if (ret == QMessageBox::Save)
//          return save();
//      else if (ret == QMessageBox::Cancel)
//          return false;
//  }
    return true;
}

void LidarWindow::SetCurrentFile(const QString& fileName)
{
	//ms_TerrainFile = QFileInfo(fileName).canonicalFilePath();
	setWindowModified(false);
	setWindowTitle(fileName + "[*]");
}

QString LidarWindow::StrippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void LidarWindow::CreateActions()
{
	mp_actionRenderAxes = new QAction(QIcon(""), tr("Render Axes"), this);
	mp_actionRenderAxes->setStatusTip(tr("Render Cartesian axes"));
	mp_actionRenderAxes->setCheckable(true);
	connect(mp_actionRenderAxes, SIGNAL(triggered()), this, SLOT(OnRenderAxes()));

	mp_actionRenderDim = new QAction(QIcon(""), tr("Dimensions"), this);
	mp_actionRenderDim->setStatusTip(tr("Render overall dimensions"));
	mp_actionRenderDim->setCheckable(true);
	connect(mp_actionRenderDim, SIGNAL(triggered()), this, SLOT(OnRenderDim()));
}

void LidarWindow::initializeGL()
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

	ResetView();

	// log opengl errors
	GLManager::CheckForOpenGLError(__FILE__, __LINE__);
}

void LidarWindow::resizeGL(int w, int h)
{
	Q_UNUSED(w);
	Q_UNUSED(h);

	// do nothing, size related settings
	// handled in paint
}

bool LidarWindow::InitializeShaders()
{
	// Build shaders and add them to GLManager.
	//

	bool shaderStatus = true;

	// load and compile the PNT (position/normal/texture) shaders
	shaderStatus = m_shaderPNT.BuildShaderFromString("PNT", ShaderLib::GetVertexShaderPNT(), ShaderLib::GetFragmentShaderPNT());

	if (shaderStatus)
	{
		// load and compile the PC (position/color) shaders
		shaderStatus = m_shaderPC.BuildShaderFromString("PC", ShaderLib::GetVertexShaderPC(), ShaderLib::GetFragmentShaderPC());
	}

	if (shaderStatus)
	{
		// load and compile the SS (screen space) shaders
		shaderStatus = m_shaderSS.BuildShaderFromString("SS", ShaderLib::GetVertexShaderSS(), ShaderLib::GetFragmentShaderSS());
	}

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	GLManager::AddShader("PNT", &m_shaderPNT);
	GLManager::AddShader("PC", &m_shaderPC);
	GLManager::AddShader("SS", &m_shaderSS);

	m_shaderPNT.SetUniform("Light.Color", vec3(1.0f, 1.0f, 1.0f));
	m_shaderPNT.SetUniform("Light.Position", vec3(0.0f, 0.0f, 0.0f));
	m_shaderPNT.SetUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
	m_shaderPNT.SetUniform("Material.Kd", 0.9f, 0.9f, 0.9f);
	m_shaderPNT.SetUniform("Material.Ks", 0.1f, 0.1f, 0.1f);
	m_shaderPNT.SetUniform("Material.Shininess", 100.0f);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	return shaderStatus;
}

void LidarWindow::ResetView()
{
	if (mp_LasFile)
	{
		// place camera above point cloud center facing down
		VEC3 pos;
		pos.x = (mp_LasFile->GetMaxX() + mp_LasFile->GetMinX()) * 0.5;
		pos.y = (mp_LasFile->GetMaxY() + mp_LasFile->GetMinY()) * 0.5;
		pos.z = (mp_LasFile->GetMaxZ() + mp_LasFile->GetMinZ()) * 0.5;
		pos.z = mp_LasFile->GetMaxZ() * 2.0;
		m_Camera.SetPos(pos);
	}
	else
	{
		// world origin
	}
	m_Camera.SetDirUp(VEC3(0, 0, -1), VEC3(0, 1, 0));

	UpdateOrthoScale();
	
	update();
}

// context menu handlers

void LidarWindow::OnRenderAxes()
{
	mb_RenderAxes = !mb_RenderAxes;
	update();
}

void LidarWindow::OnRenderDim()
{
	mb_RenderDim = !mb_RenderDim;
	update();
}
