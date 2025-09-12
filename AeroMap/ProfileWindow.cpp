// ProfileWindow.cpp
// Show path profile.
//

#include <QGraphicsScene>
#include <QGraphicsTextItem>

#include "PngFile.h"
#include "MainWindow.h"
#include "ProfileWindow.h"

const int MIN_WIN_SIZE = 9;

const double VBUF = 0.1;		// vertical margin around plot, where 1.0 == window height
const double HBUF_L = 0.1;		// horizontal margin around plot, whre 1.0 == window width
const double HBUF_R = 0.05;

ProfileWindow::ProfileWindow(QWidget* parent)
	: QGraphicsView(parent)
	, mp_actionScaleFull(nullptr)
	, mp_actionScaleRange(nullptr)
	, m_DataPitch(0.0)
	, m_minElev(0.0)
	, m_maxElev(0.0)
	, mb_ScaleFull(true)
	, mb_DebugInfo(true)
{
	setWindowTitle(tr("Path Profile"));
	setWindowFlag(Qt::Window);
	setWindowFlag(Qt::WindowTitleHint);
	setWindowFlag(Qt::WindowCloseButtonHint);
	setWindowFlag(Qt::WindowStaysOnTopHint);
	setWindowFlag(Qt::WindowMinimizeButtonHint, false);
	setWindowFlag(Qt::WindowMaximizeButtonHint, false);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setAttribute(Qt::WA_DeleteOnClose);
	setFocusPolicy(Qt::FocusPolicy::ClickFocus);
	setMinimumSize(MIN_WIN_SIZE, MIN_WIN_SIZE);
	resize(600, 200);

	setMouseTracking(true);

	// set up fonts

	m_Font.setFamily("Consolas");
	m_Font.setPointSize(10.0);
	m_Font.setItalic(false);
	
	CreateActions();
}

ProfileWindow::~ProfileWindow()
{
	delete mp_actionScaleFull;
	delete mp_actionScaleRange;
}

void ProfileWindow::Render()
{
	// Draw profile across window.
	//

	m_scene.clear();
	m_scene.setSceneRect(0.0, 0.0, width() - 1, height() - 1);
	//m_scene.setBackgroundBrush(brushBack);

	// plot
	QPen penLine(QColor(0, 0, 0), Qt::SolidLine);
	double winWidth = width() * (1.0 - HBUF_L - HBUF_R);
	double winHeight = height() * (1.0 - 2.0 * VBUF);
	double dx = winWidth / (double)m_Data.size();
	double x = HBUF_L * width();
	for (int i = 1; i < m_Data.size(); ++i)
	{
		double y0 = m_Data[i - 1];
		double y1 = m_Data[i];
		if (mb_ScaleFull == true)
		{
			y0 = VBUF * height() + (y0 / m_maxElev) * winHeight;
			y1 = VBUF * height() + (y1 / m_maxElev) * winHeight;
		}
		else
		{
			y0 = VBUF * height() + ((y0 - m_minElev) / (m_maxElev - m_minElev)) * winHeight;
			y1 = VBUF * height() + ((y1 - m_minElev) / (m_maxElev - m_minElev)) * winHeight;
		}
		y0 = height() - y0;
		y1 = height() - y1;
		m_scene.addLine(x, y0, x + dx, y1, penLine);
		x += dx;
	}

	// scale
	QPen penScale(QColor(120, 120, 120), Qt::SolidLine);
	double y = VBUF * height();
	char buf[32];
	sprintf(buf, "%0.1f", m_maxElev);
	QGraphicsTextItem* pItem = m_scene.addText(buf);
	QRectF rect = pItem->boundingRect();
	pItem->setPos(1, y - rect.height() / 2.0);
	m_scene.addLine(rect.width() + 2, y, (HBUF_L * width())-2, y, penScale);
	if (mb_ScaleFull == false)
	{
		// from min to max
		const int STEPS = 4;
		for (int i = 0; i < STEPS; ++i)
		{
			double factor = (1.0 / (double)STEPS);
			y += winHeight * factor;
			double elev = m_maxElev - (i + 1) * factor * (m_maxElev-m_minElev);
			m_scene.addLine(rect.width() + 2, y, (HBUF_L * width()) - 2, y, penScale);

			sprintf(buf, "%0.1f", elev);
			QGraphicsTextItem* pItem = m_scene.addText(buf);
			pItem->setPos(1, y - rect.height() / 2.0);
		}
	}
	else
	{
		// from zero up
		y = height() * (1.0-VBUF);
		m_scene.addLine(rect.width() + 2, y, (HBUF_L * width()) - 2, y, penScale);

		sprintf(buf, "%0.1f", 0.0);
		QGraphicsTextItem* pItem = m_scene.addText(buf);
		pItem->setPos(1, y - rect.height() / 2.0);
	}

	setScene(&m_scene);
}

void ProfileWindow::SetData(double dx, std::vector<double> data)
{
	// Set path data.
	//
	// Inputs:
	//		dx   = fixed horizontal spacing between points
	//		data = data points
	//

	assert(dx > 0.0);
	assert(data.size() > 0);

	m_scene.clear();

	m_Data = data;
	m_DataPitch = dx;

	// get range
	m_minElev = m_Data[0];
	m_maxElev = m_Data[0];
	for (int i = 1; i < m_Data.size(); ++i)
	{
		if (m_Data[i] < m_minElev)
			m_minElev = m_Data[i];
		if (m_Data[i] > m_maxElev)
			m_maxElev = m_Data[i];
	}

	Render();
}

void ProfileWindow::PixelToWorld(int xp, double& x, double& y)
{
	// Map window x/y values to world xy values for current ortho view.
	//
	// Inputs:
	//		xp = x pixel value
	//
	// Outputs:
	//		x, y = 2d world values
	//

	x = 0.0;
	y = 0.0;

	if (xp >= 0 && xp < m_Data.size())
	{
		x = xp * m_DataPitch;
		y = m_Data[xp];
	}
}

void ProfileWindow::mouseDoubleClickEvent(QMouseEvent* event)
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
}

void ProfileWindow::mousePressEvent(QMouseEvent* event)
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

	VEC2 pos;
	PixelToWorld(event->x(), pos.x, pos.y);

	switch (GetApp()->m_Tool.GetTool()) {
	case Tool::ToolType::Select:
		break;
	default:   // no active tool
		break;
	}

	m_ptLastMouse.x = event->x();  // last recorded mouse position
	m_ptLastMouse.y = event->y();
}

void ProfileWindow::mouseReleaseEvent(QMouseEvent* /* event */)
{
	// This event handler can be reimplemented in a subclass to receive 
	// mouse release events for the widget.
	//

	switch (GetApp()->m_Tool.GetTool()) {
	case Tool::ToolType::Select:   // finished selecting
		//mb_Selecting = false;
		break;
	default:
		break;
	}
}

void ProfileWindow::mouseMoveEvent(QMouseEvent* event)
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

	double worldX = 0.0;
	double worldY = 0.0;

	// convert current mouse position to world units
	PixelToWorld(event->x(), worldX, worldY);

	if (event->buttons() & Qt::LeftButton)		// left button is depressed
	{
	}       // if left button

	m_ptLastMouse.x = event->x();
	m_ptLastMouse.y = event->y();
}

void ProfileWindow::wheelEvent(QWheelEvent* event)
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
	}
	else
	{
		event->ignore();
	}
}

void ProfileWindow::keyPressEvent(QKeyEvent* event)
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

	// make move deltas relative to size of workspace
	//double delta = 1.0;
	//double delta = mp_Terrain->GetRadius() * 0.01;

	switch (event->key()) {
	case Qt::Key_A:
		break;
	case Qt::Key_D:
		mb_DebugInfo = !mb_DebugInfo;
		break;
	case Qt::Key_H:
		// "Home", reset view
		break;
	case Qt::Key_S:
		break;
	case Qt::Key_W:
		break;
	case Qt::Key_Up:
		break;
	case Qt::Key_Down:
		break;
	case Qt::Key_Left:
		break;
	case Qt::Key_Right:
		break;
	case Qt::Key_PageUp:
		break;
	case Qt::Key_PageDown:
		break;
	}

	__super::keyPressEvent(event);
}

void ProfileWindow::keyReleaseEvent(QKeyEvent* event)
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

void ProfileWindow::closeEvent(QCloseEvent* event)
{
	//if (MaybeSave())
	{
        event->accept();
    }
    //else
    //{
        //event->ignore();
    //}
}

void ProfileWindow::contextMenuEvent(QContextMenuEvent* event)
{
	// check selected items
	mp_actionScaleFull->setChecked(mb_ScaleFull);
	mp_actionScaleRange->setChecked(!mb_ScaleFull);

	QMenu menu(this);
	menu.addAction(mp_actionScaleFull);
	menu.addAction(mp_actionScaleRange);

	menu.exec(event->globalPos());
}

void ProfileWindow::CreateActions()
{
	mp_actionScaleFull = new QAction(QIcon(""), tr("Scale Full"), this);
	mp_actionScaleFull->setStatusTip(tr("Scale from ground to elevation values"));
	mp_actionScaleFull->setCheckable(true);
	verify_connect(mp_actionScaleFull, SIGNAL(triggered()), this, SLOT(OnScaleFull()));

	mp_actionScaleRange = new QAction(QIcon(""), tr("Scale Range"), this);
	mp_actionScaleRange->setStatusTip(tr("Scale from min to max elevation values"));
	mp_actionScaleRange->setCheckable(true);
	verify_connect(mp_actionScaleRange, SIGNAL(triggered()), this, SLOT(OnScaleRange()));
}

// context menu handlers

void ProfileWindow::OnScaleFull()
{
	mb_ScaleFull = true;
	Render();
}

void ProfileWindow::OnScaleRange()
{
	mb_ScaleFull = false;
	Render();
}
