// OrthoWindow.cpp
// View/analyse orthophotos.
//

#include "AeroLib.h"
#include "MainWindow.h"
#include "MetaDataDlg.h"
#include "OrthoWindow.h"

#include <QStatusBar>

OrthoWindow::OrthoWindow(QWidget* parent)
	: QGraphicsView(parent) 
	, mp_actionRenderDim(nullptr)
	, mp_actionClear(nullptr)
	, mp_actionProperties(nullptr)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setMinimumSize(50, 50);

	mp_Parent = static_cast<MainWindow*>(parent);
	assert(mp_Parent != nullptr);

	InitQuad(m_colorBACK, 0.0F, 0.0F, 0.0F);
	InitQuad(m_colorTEXT, 0.9F, 0.9F, 0.9F);
	InitQuad(m_colorDIM, 0.9F, 0.9F, 0.9F);

	setMouseTracking(true);
	setAcceptDrops(true);
	grabKeyboard();

	QBrush brushBack(SCENE_BACK_COLOR, Qt::SolidPattern);
	m_scene.setBackgroundBrush(brushBack);

	// set up fonts
	m_Font.setFamily("Consolas");
	m_Font.setPointSize(10.0);
	m_Font.setItalic(false);

	CreateActions();

	LoadOrthophoto();
}

OrthoWindow::~OrthoWindow()
{
	delete mp_actionRenderDim;
	delete mp_actionClear;
	delete mp_actionProperties;
}

void OrthoWindow::LoadOrthophoto()
{
	if (AeroLib::FileExists(tree.orthophoto_tif) == false)
	{
		Logger::Write(__FUNCTION__, "Orthophoto not found: '%s'", tree.orthophoto_tif.c_str());
		return;
	}

	QPixmap pixmap;
	if (pixmap.load(tree.orthophoto_tif.c_str()) == false)
	{
		Logger::Write(__FUNCTION__, "Unable to load orthophoto: '%s'", tree.orthophoto_tif.c_str());
		return;
	}

	// load orthophoto extents
	if (AeroLib::FileExists(tree.orthophoto_corners) == false)
	{
		Logger::Write(__FUNCTION__, "Orthophoto extents not found: '%s'", tree.orthophoto_corners.c_str());
	}
	else
	{
		FILE* pFile = fopen(tree.orthophoto_corners.c_str(), "rt");
		if (pFile)
		{
			char buf[255] = { 0 };
			fgets(buf, sizeof(buf), pFile);
			fclose(pFile);

			XString str = buf;
			int token_count = str.Tokenize(", \t");
			if (token_count >= 4)
			{
				m_ext.x0 = str.GetToken(0).GetDouble();
				m_ext.y0 = str.GetToken(1).GetDouble();
				m_ext.x1 = str.GetToken(2).GetDouble();
				m_ext.y1 = str.GetToken(3).GetDouble();
			}
		}
	}

	m_scene.addPixmap(pixmap);
	setScene(&m_scene);
}

void OrthoWindow::ResetView()
{
	// reset current transform before scaling
	QTransform transform;
	transform.reset();
	setTransform(transform);

	QRect rect_win = rect();
	QRectF rect_scene = sceneRect();

	double sx = (double)rect_win.width() / rect_scene.width();
	double sy = (double)rect_win.height() / rect_scene.height();
	double sf = std::min(sx, sy);
	
	// leave small buffer around image
	sf *= 0.96;

	scale(sf, sf);

	UpdateGraphics();
}

void OrthoWindow::AddDistancePoint(PointD pt)
{
	// Inputs:
	//		pt = world coordinates
	//

	DistanceType dist;
	dist.x = pt.x;
	dist.y = pt.y;
	if (m_dist_pts.size() > 0)
	{
		PointD pt_scene0 = WorldToScene(m_dist_pts.back().x, m_dist_pts.back().y);
		PointD pt_scene1 = WorldToScene(pt.x, pt.y);
		
		dist.pLine = m_scene.addLine(pt_scene0.x, pt_scene0.y, pt_scene1.x, pt_scene1.y);

		double distance = Distance(pt.x, pt.y, m_dist_pts.back().x, m_dist_pts.back().y);
		dist.pText = m_scene.addText(XString::Format("%0.1f", distance).c_str());
		dist.pText->setPos(pt_scene1.x, pt_scene1.y);
	}
	m_dist_pts.push_back(dist);

	UpdateGraphics();
}

void OrthoWindow::ClearDistancePoints()
{
	for (auto d : m_dist_pts)
	{
		if (d.pLine)
			m_scene.removeItem(d.pLine);
		if (d.pText)
			m_scene.removeItem(d.pText);
	}
	m_dist_pts.clear();
}

void OrthoWindow::UpdateGraphics()
{
	double sf = transform().m11();

	for (auto d : m_dist_pts)
	{
		if (d.pLine)
		{
			double line_width = 1.0 / sf;

			QPen pen(QColor(10, 200, 10), line_width, Qt::SolidLine);
			d.pLine->setPen(pen);
		}
		if (d.pText)
		{
			double point_size = 1.0 / sf;
			point_size *= 10.0;

			m_Font.setPointSizeF(point_size);
			d.pText->setFont(m_Font);

			PointD pt_scene = WorldToScene(d.x, d.y);
			d.pText->setPos(pt_scene.x, pt_scene.y);
		}
	}
}

PointD OrthoWindow::PixelToWorld(int xp, int yp)
{
	// Map from window coordinates to world
	// coordinates.
	//

	PointD pt;

	QRectF rect_scene = sceneRect();
	QPointF pt_scene = mapToScene(QPoint(xp, yp));

	double dx = pt_scene.x() / rect_scene.width();
	double dy = pt_scene.y() / rect_scene.height();

	pt.x = Lerp(dx, m_ext.x0, m_ext.x1);
	pt.y = Lerp(dy, m_ext.y1, m_ext.y0);

	return pt;
}

PointType OrthoWindow::WorldToPixel(double x, double y)
{
	PointType pt;

	PointD pt_scene = WorldToScene(x, y);
	QPoint pt_win = mapFromScene(QPointF(pt_scene.x, pt_scene.y));

	pt.x = pt_win.x();
	pt.y = pt_win.y();

	return pt;
}

PointD OrthoWindow::WorldToScene(double x, double y)
{
	// Map world units to scene units.
	// (scene dim is same as backing image dim).
	//

	PointD pt;

	double dx = (x - m_ext.x0) / m_ext.DX();
	double dy = (y - m_ext.y0) / m_ext.DY();
	dy = 1.0 - dy;

	QRectF rect_scene = sceneRect();

	double scene_x = rect_scene.width() * dx;
	double scene_y = rect_scene.height() * dy;

	pt.x = scene_x;
	pt.y = scene_y;

	return pt;
}

void OrthoWindow::CreateActions()
{
	mp_actionRenderDim = new QAction(QIcon(""), tr("Render Dimensions"), this);
	mp_actionRenderDim->setStatusTip(tr("Render dimensions"));
	mp_actionRenderDim->setCheckable(true);
	connect(mp_actionRenderDim, SIGNAL(triggered()), this, SLOT(OnRenderDim()));

	mp_actionClear = new QAction(QIcon(""), tr("Clear"), this);
	mp_actionClear->setStatusTip(tr("Clear measurement artifacts"));
	connect(mp_actionClear, SIGNAL(triggered()), this, SLOT(OnClear()));

	mp_actionProperties = new QAction(QIcon(""), tr("Properties"), this);
	mp_actionProperties->setStatusTip(tr("Display orthophoto properties"));
	connect(mp_actionProperties, SIGNAL(triggered()), this, SLOT(OnProperties()));
}

void OrthoWindow::closeEvent(QCloseEvent* event)
{
	Q_UNUSED(event);
}

void OrthoWindow::contextMenuEvent(QContextMenuEvent* event)
{
	// check selected items

	mp_actionRenderDim->setChecked(mb_RenderDim);

	QMenu menu(this);
	menu.addAction(mp_actionRenderDim);
	menu.addAction(mp_actionClear);
	menu.addAction(mp_actionProperties);
	menu.exec(event->globalPos());
}

void OrthoWindow::mouseDoubleClickEvent(QMouseEvent* event)
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

void OrthoWindow::mouseMoveEvent(QMouseEvent* event)
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

	PointD pt_world = PixelToWorld(event->x(), event->y());

	if (event->buttons() & Qt::LeftButton)		// left button is depressed
	{
		update();
	}       // if left button

	PointD pt_scene = WorldToScene(pt_world.x, pt_world.y);
	PointType pt_test = WorldToPixel(pt_world.x, pt_world.y);

	XString str = XString::Format("Mouse: (%d, %d)  World: (%0.1f, %0.1f)  Scene: (%0.1f, %0.1f)  Map Back: (%d, %d)", 
		event->x(), event->y(), pt_world.x, pt_world.y, pt_scene.x, pt_scene.y, pt_test.x, pt_test.y);
	mp_Parent->statusBar()->showMessage(str.c_str());

	m_ptLastMouse.x = event->x();
	m_ptLastMouse.y = event->y();
}

void OrthoWindow::mousePressEvent(QMouseEvent* event)
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

	if (event->buttons() & Qt::LeftButton)		// left button is depressed
	{
		switch (GetApp()->m_Tool.GetTool()) {
		case Tool::ToolType::Select:
		case Tool::ToolType::ViewZoom:
		case Tool::ToolType::ViewPan:
			m_ptAnchor.x = event->x();
			m_ptAnchor.y = event->y();
			break;
		case Tool::ToolType::Distance:
		{
			PointD pt = PixelToWorld(event->x(), event->y());
			if (m_ext.Contains(pt.x, pt.y))
				AddDistancePoint(pt);
		}
		break;
		case Tool::ToolType::Area:
			break;
		default:   // no active tool
			break;
		}
	}

	update();

	m_ptLastMouse.x = event->x();  // last recorded mouse position
	m_ptLastMouse.y = event->y();
}

void OrthoWindow::mouseReleaseEvent(QMouseEvent* event)
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

	m_ptLastMouse.x = event->x();  // last recorded mouse position
	m_ptLastMouse.y = event->y();
}

void OrthoWindow::wheelEvent(QWheelEvent* event)
{
	// This event handler, for event event, can be reimplemented in a subclass to receive wheel events
	// for the widget.
	//
	// If you reimplement this handler, it is very important that you ignore() the event if you do not
	// handle it, so that the widget's parent can interpret it.
	//
	// The default implementation ignores the event.
	//

	int delta = event->angleDelta().y();
	if (delta)
	{
		double sf = 1.0;
		if (delta > 0)
			sf = 0.95;
		else if (delta < 0)
			sf = 1.05;
		scale(sf, sf);

		UpdateGraphics();
		update();
	}
	else
	{
		event->ignore();
	}
}

void OrthoWindow::keyPressEvent(QKeyEvent* event)
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

void OrthoWindow::keyReleaseEvent(QKeyEvent* event)
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

void OrthoWindow::showEvent(QShowEvent* event)
{
	Q_UNUSED(event);

	ResetView();
}

void OrthoWindow::OnRenderDim()
{
	mb_RenderDim = !mb_RenderDim;
}

void OrthoWindow::OnClear()
{
	ClearDistancePoints();
}

void OrthoWindow::OnProperties()
{
	XString str = XString::Format("File: %s\n", tree.orthophoto_tif.c_str());
	str += XString::Format("X Extents: %0.3f -> %0.3f (%0.2f)\n", m_ext.x0, m_ext.x1, m_ext.DX());
	str += XString::Format("Y Extents: %0.3f -> %0.3f (%0.2f)\n", m_ext.y0, m_ext.y1, m_ext.DY());

	MetaDataDlg dlg(this, "Orthophoto Properties");
	dlg.SetMetaData(str);
	dlg.exec();
}
