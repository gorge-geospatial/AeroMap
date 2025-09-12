#ifndef LIDARWINDOW_H
#define LIDARWINDOW_H

#include "AeroMap.h"	        // application header
#include "LasFile.h"			// LAS/LAZ file manager
#include "Camera.h"
#include "ShaderGL.h"
#include "VertexBufferGL.h"
#include "ScaleColor.h"

#include <QOpenGLWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>

#include <stack>

class MainWindow;

class LidarWindow : public QOpenGLWidget
{
    Q_OBJECT

public:

	enum class LidarAttr
	{
		None,
		Z,				// z value
		Return,			// return number
		Class,			// point classification
	};

public:
	
	LidarWindow(QWidget* parent, int fileIndex);
	~LidarWindow();

	bool Save();
	bool SaveAs();
	bool SaveFile(const QString& fileName);

	void SetColorScale(LidarAttr scaleAttr, const char* scaleFile);
	void ResetView();

	LasFile* GetLasFile();

protected:

	// QGLWidget
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;

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

	int m_LasFileIndex;			// index of las file being rendered
	LasFile* mp_LasFile;		// ptr to project copy of las file

	Camera m_Camera;			// 3D camera
	QFont m_Font;				// default font

	ShaderGL m_shaderPNT;		// shader program for PNT type vertices
	ShaderGL m_shaderPC;		// shader program for PC type vertices
	ShaderGL m_shaderSS;		// screen space shader

	VertexBufferGL* mp_VB;		// point cloud vertex buffer

	double mf_Height;				// dimensions of ortho view windows, world units
	double mf_Width;

	mat4 m_matModel;				// current matrices
	mat4 m_matView;
	mat4 m_matProjection;

	SizeType	m_winSize;			// client area dimensions
	PointType  	m_ptAnchor;			// anchor point for current operation
	PointType	m_ptLastMouse;		// last recorded mouse position
	RectType   	m_rctSel;			// current 2d selection

	bool mb_RenderAxes;				// render cartesian axes
	bool mb_RenderDim;				// render overall dimensions
	bool mb_Selecting;				// selecting vertices
	bool mb_DebugInfo;				// render add'l debug info

	ScaleColor* mp_ScaleColor;		// active color scale
	LidarAttr m_ScaleAttr;			// attribute color scale applied to

	float m_colorBACK[4];			// background color
	float m_colorTEXT[4];
	float m_colorDIM[4];

	float m_colorAXIS_X[4];
	float m_colorAXIS_Y[4];
	float m_colorAXIS_Z[4];

	// context menu actions
	QAction* mp_actionRenderAxes;
	QAction* mp_actionRenderDim;

	MainWindow* mp_Parent;

private slots:

	void documentWasModified();

	void OnRenderAxes();
	void OnRenderDim();

private:

	void RenderAxes();
	void RenderDistance(QPainter* pPainter);
	void RenderSelectBox();
	void RenderText(QPainter* pPainter);

	// transformations
	void PixelToWorld(int xp, int yp, double& worldX, double& worldY);
	bool WorldToPixel(double worldX, double worldY, int &xp, int &yp);

	void CreateVertexBuffer();
	void DeleteVertexBuffer();
	void SetupMatrices();
	bool InitializeShaders();
	void UpdateOrthoScale();

	bool maybeSave();
	void SetCurrentFile(const QString& fileName);
	QString StrippedName(const QString& fullFileName);

	// lighting attributes
	void SetBackColor(UInt8 R, UInt8 G, UInt8 B);
	void GetBackColor(UInt8& R, UInt8& G, UInt8& B);

	void CreateActions();
};

#endif // #ifndef LIDARWINDOW_H
