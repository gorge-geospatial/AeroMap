#ifndef PROFILEWINDOW_H
#define PROFILEWINDOW_H

#include "AeroMap.h"	        // application header

#include <QGraphicsView>
#include <QMessageBox>
#include <QMenu>

class MainWindow;

class ProfileWindow : public QGraphicsView
{
    Q_OBJECT

public:
	
	ProfileWindow(QWidget* parent);
	~ProfileWindow();

	void SetData(double dx, std::vector<double> data);

protected:

	// QWidget
	virtual void closeEvent(QCloseEvent* event) override;
	virtual void contextMenuEvent(QContextMenuEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

private:

	std::vector<double> m_Data;		// elevation data
	double m_DataPitch;				// fixed spacing between m_Data values
	double m_minElev;				// elevation range
	double m_maxElev;

	QGraphicsScene m_scene;			// backing scene
	QFont m_Font;					// default font

	SizeType	m_winSize;			// client area dimensions
	PointType	m_ptLastMouse;		// last recorded mouse position

	bool mb_ScaleFull;				// scale from ground up (else min/max)
	bool mb_DebugInfo;

	// context menu actions
	QAction* mp_actionScaleFull;
	QAction* mp_actionScaleRange;

private slots:

	void OnScaleFull();
	void OnScaleRange();

private:

	void Render();

	// coordinate mapping
	void PixelToWorld(int xp, double& x, double& y);

	void CreateActions();
};

#endif // #ifndef PROFILEWINDOW_H
