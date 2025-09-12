// DroneWindow.cpp
// Photogrammetry workspace.
//

#include "ShaderLib.h"
#include "GLManager.h"
#include "MainWindow.h"
#include "DroneWindow.h"

#include <QStatusBar>

const int THUMB_WIDTH = 136;
const int THUMB_MARGIN = 12;

DroneWindow::DroneWindow(QWidget* parent)
	: QOpenGLWidget(parent)
	, mb_Selecting(false)
	, mb_DebugInfo(true)
	, m_ThumbScale(0.0)
	, m_ThumbHeight(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setMinimumSize(50, 50);

	mp_Parent = static_cast<MainWindow*>(parent);
	assert(mp_Parent != nullptr);

	InitQuad(m_colorBACK, 0.0F, 0.0F, 0.0F);
	InitQuad(m_colorTEXT, 0.9F, 0.9F, 0.9F);

	setMouseTracking(true);
	setAcceptDrops(true);
	grabKeyboard();

	// set up fonts

	m_Font.setFamily("Consolas");
	m_Font.setPointSize(10.0);
	m_Font.setItalic(false);
	
	CreateActions();
}

DroneWindow::~DroneWindow()
{

	// clean up opengl state, other gl windows will
	// re-inititialize it
	GLManager::Shutdown();
}

void DroneWindow::paintGL()
{
	if (GLManager::IsInitialized() == false)
		return;

	if (GetImageCount() == 0)
		LoadImageList();

	QPainter painter;
	if (!painter.begin(this))
		assert(false);
	painter.beginNativePainting();

	m_winSize.cx = width();
	m_winSize.cy = height();

	// setup viewport, projection etc.:
	glViewport(0, 0, (GLint)m_winSize.cx, (GLint)m_winSize.cy);

	// clear back buffer

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	m_shaderImage.Activate();

	// set up screen space projection
	mat4 matProj = glm::ortho(0.0F, (float)(m_winSize.cx - 1), (float)(m_winSize.cy - 1), 0.0F);
	m_shaderImage.SetUniform("ProjectionMatrix", matProj);

	int xoff = THUMB_MARGIN;
	int yoff = THUMB_MARGIN;
	for (ImageType image : m_ImageList)
	{
		m_shaderImage.SetUniform("xoff", (float)xoff);
		m_shaderImage.SetUniform("yoff", (float)yoff);

		image.pVB->Render();

		xoff += THUMB_WIDTH + THUMB_MARGIN;
		if (xoff + THUMB_WIDTH + THUMB_MARGIN > m_winSize.cx)
		{
			xoff = THUMB_MARGIN;
			yoff += m_ThumbHeight + THUMB_MARGIN;
		}
	}

	RenderSelectBox();
	RenderText(&painter);

	painter.endNativePainting();
	painter.end();

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);
}

int DroneWindow::LoadImageList()
{
	QDir dir(GetProject().GetDroneInputPath().c_str());
	dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);

	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i)
	{
		QFileInfo fileInfo = list.at(i);

		XString file_name = fileInfo.fileName().toLatin1().constData();
		XString file_path = fileInfo.absoluteFilePath().toLatin1().constData();

		if (fileInfo.isFile())
		{
			if (Photo::IsSupportedImageFormat(file_name) == false)
				continue;

			QImage image;
			if (image.load(file_path.c_str()) == false)
			{
				Logger::Write(__FUNCTION__, "Unable to load image file: '%s'", file_path.c_str());
				continue;
			}

			if (m_ThumbScale == 0.0)
			{
				m_ThumbScale = (double)image.width() / (double)THUMB_WIDTH;
				m_ThumbHeight = (int)round((double)image.height() / m_ThumbScale);
			}

			ImageType entry;
			entry.file_name = file_path;

			std::vector<VertexBufferGL::VertexSSC> vxList;
			VertexBufferGL::VertexSSC vx;

			int destx = 0;
			for (double x = 0; x < image.width(); x += m_ThumbScale)
			{
				int desty = 0;
				for (double y = 0; y < image.height(); y += m_ThumbScale)
				{
					QColor c = image.pixelColor((int)round(x), (int)round(y));
					vx.r = c.redF();
					vx.g = c.greenF();
					vx.b = c.blueF();
					vx.x = destx;
					vx.y = desty++;
					vxList.push_back(vx);
				}
				destx++;
			}

			if (vxList.size() > 0)
			{
				entry.pVB = new VertexBufferGL();
				entry.pVB->CreateSSC(GL_POINTS, vxList);
			}

			m_ImageList.push_back(entry);
		}
//TODO:
//find faster method so at least have option of loading all images
		if (GetImageCount() > 3)
			break;
	}

	return GetImageCount();
}

int DroneWindow::GetImageCount()
{
	return (int)m_ImageList.size();
}

void DroneWindow::RenderSelectBox()
{
	//	render select box
	//

	if (!mb_Selecting)					// don't render box unless we're actively selecting vertices
		return;

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

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
}

void DroneWindow::RenderText(QPainter* pPainter)
{
	Q_UNUSED(pPainter);

	//glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);

	//m_shaderSS.Activate();

	//// set up screen space projection
	//mat4 matProj = glm::ortho(0.0F, (float)(m_winSize.cx - 1), (float)(m_winSize.cy - 1), 0.0F);
	//m_shaderSS.SetUniform("ProjectionMatrix", matProj);

	//// render any text items that were added to the list on this frame
	//QPen penOld = pPainter->pen();
	//pPainter->setPen(Qt::black);
	//pPainter->setFont(m_Font);

	//int yoffset = 0;
	//char buf[80] = { 0 };
	//sprintf(buf, "Photogrammetry");
	//pPainter->drawText(2, yoffset += 12, buf);

	//if (mb_DebugInfo)
	//{
	//}

	//pPainter->setPen(penOld);
}

void DroneWindow::mouseDoubleClickEvent(QMouseEvent* event)
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

void DroneWindow::mousePressEvent(QMouseEvent* event)
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

void DroneWindow::mouseReleaseEvent(QMouseEvent* /* event */)
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

void DroneWindow::mouseMoveEvent(QMouseEvent* event)
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

	if (event->buttons() & Qt::LeftButton)		// left button is depressed
	{
		switch (GetApp()->m_Tool.GetTool()) {
		case Tool::ToolType::Select:
		case Tool::ToolType::ViewZoom:
			break;
		case Tool::ToolType::ViewPan:
			break;
		case Tool::ToolType::None:     // no tool selected
			break;
		default:
			break;
		}

		update();
	}       // if left button

	m_ptLastMouse.x = event->x();
	m_ptLastMouse.y = event->y();
}

void DroneWindow::wheelEvent(QWheelEvent* event)
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
		update();
	}
	else
	{
		event->ignore();
	}
}

void DroneWindow::keyPressEvent(QKeyEvent* event)
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
		break;
	case Qt::Key_W:
		break;
	}

	update();

	__super::keyPressEvent(event);
}

void DroneWindow::keyReleaseEvent(QKeyEvent* event)
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

void DroneWindow::SetBackColor(UInt8 R, UInt8 G, UInt8 B)
{
	m_colorBACK[0] = (float)R / 255.0F;
	m_colorBACK[1] = (float)G / 255.0F;
	m_colorBACK[2] = (float)B / 255.0F;

	glClearColor(m_colorBACK[0], m_colorBACK[1], m_colorBACK[2], 1.0);

	update();
}

void DroneWindow::GetBackColor(UInt8& R, UInt8& G, UInt8& B)
{
	R = (UInt8)(m_colorBACK[0] * 255.0);
	G = (UInt8)(m_colorBACK[1] * 255.0);
	B = (UInt8)(m_colorBACK[2] * 255.0);
}

void DroneWindow::closeEvent(QCloseEvent* event)
{
    event->accept();
}

void DroneWindow::contextMenuEvent(QContextMenuEvent* event)
{
	Q_UNUSED(event);
}

void DroneWindow::CreateActions()
{
}

void DroneWindow::initializeGL()
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

void DroneWindow::resizeGL(int w, int h)
{
	Q_UNUSED(w);
	Q_UNUSED(h);

	// do nothing, size related settings
	// handled in paint
}

bool DroneWindow::InitializeShaders()
{
	// Build shaders and add them to GLManager.
	//

	bool shaderStatus = m_shaderSS.BuildShaderFromString("SS", ShaderLib::GetVertexShaderSS(), ShaderLib::GetFragmentShaderSS());
	if (shaderStatus)
		shaderStatus = m_shaderImage.BuildShaderFromString("Image", ShaderLib::GetVertexShaderImage(), ShaderLib::GetFragmentShaderImage());

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	return shaderStatus;
}

void DroneWindow::ResetView()
{
	update();
}
