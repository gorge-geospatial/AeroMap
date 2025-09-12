#ifndef ORTHOWINDOW_H
#define ORTHOWINDOW_H

#include "AeroMap.h"	        // application header

#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QMessageBox>
#include <QMenu>

class MainWindow;

class OrthoWindow : public QGraphicsView
{
    Q_OBJECT

public:
	
	OrthoWindow(QWidget* parent);
	~OrthoWindow();

	void ResetView();

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
	virtual void showEvent(QShowEvent* event) override;

private:

	QGraphicsScene m_scene;			// backing scene
	RectD m_ext;					// extents of orthoptho, world units
	QFont m_Font;					// default font

	SizeType	m_winSize;			// client area dimensions
	PointType  	m_ptAnchor;			// anchor point for current operation
	PointType	m_ptLastMouse;		// last recorded mouse position
	RectType   	m_rctSel;			// current 2d selection

	bool mb_RenderDim;				// render overall dimensions
	bool mb_Selecting;				// selecting vertices
	bool mb_DebugInfo;				// render add'l debug info

	float m_colorBACK[4];			// background color
	float m_colorTEXT[4];
	float m_colorDIM[4];

	// context menu actions
	QAction* mp_actionRenderDim;
	QAction* mp_actionClear;
	QAction* mp_actionProperties;

	MainWindow* mp_Parent;

	struct DistanceType
	{
		double x, y;					// position, world units
		QGraphicsLineItem* pLine;		
		QGraphicsTextItem* pText;

		DistanceType()
			: x(0.0), y(0.0)
			, pLine(nullptr), pText(nullptr)
		{
		}
	};
	std::vector<DistanceType> m_dist_pts;

private slots:

	void OnRenderDim();
	void OnClear();
	void OnProperties();

private:

	// transformations
	PointD PixelToWorld(int xp, int yp);
	PointType WorldToPixel(double x, double y);
	PointD WorldToScene(double x, double y);

	void CreateActions();
	void LoadOrthophoto();
	void UpdateGraphics();

	void AddDistancePoint(PointD pt);
	void ClearDistancePoints();
};

#endif // #ifndef ORTHOWINDOW_H
