#ifndef TERRAINWINDOW_H
#define TERRAINWINDOW_H

#include "AeroMap.h"	        // application header
#include "Camera.h"
#include "ShaderGL.h"
#include "TerrainGL.h"			// backing terrain
#include "VertexBufferGL.h"
#include "Tool.h"				// tool manager
#include "ProfileWindow.h"

#include <QOpenGLWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>

constexpr char* ORTHO_KEY = "Orthophoto"; 

class MainWindow;

class TerrainWindow : public QOpenGLWidget
{
    Q_OBJECT

public:
	
	TerrainWindow(QWidget* parent, int model_type);
	~TerrainWindow();

	bool Save();
	bool ConfirmClose();
	QString UserFriendlyCurrentFile();
	QString CurrentFile() { return ms_FileName.c_str(); }
	QString GetCurrentFile();

	void SetColorScale(XString scale);
	void ResetView();

	void Contour();
	void OnElevation();

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

	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
	virtual void dragMoveEvent(QDragMoveEvent* event) override;
	virtual void dropEvent(QDropEvent* event) override;

private:

	enum class View
	{
		Top,		// ortho top view
		Front,		// ortho front view
		Side,		// ortho side
		ThreeD,		// 3d ground view
		Count
	};

	struct TextType
	{
		int x;
		int y;
		char text[80];
	};
	std::vector<TextType> mv_Text;

	XString		ms_FileName;		// path to terrain model
	TerrainGL*	mp_Terrain;			// backing terrain being rendered/edited
	View		m_ViewID;			// "this" view's ID

	Camera m_Camera;				// 3D camera
	QFont m_Font;					// default font
	QTimer m_timerSim;				// simulation timer

	ShaderGL m_shaderPNT;			// shader program for PNT type vertices
	ShaderGL m_shaderPNC;			// shader program for PNC type vertices
	ShaderGL m_shaderPC;			// shader program for PC type vertices
	ShaderGL m_shaderPCT;			// shader program for PCT type vertices
	ShaderGL m_shaderSS;			// screen space shader

	double mf_Height;				// dimensions of ortho view windows, world units
	double mf_Width;
	bool mb_View;

//TODO:
//hardcoded to include skybox - need to work out general solution
	const double NEAR_PLANE = 1.0;		// near clipping plane
	const double FAR_PLANE = 50000.0;	// far clipping plane

	mat4 m_matModel;				// current matrices
	mat4 m_matView;
	mat4 m_matProjection;

	FILL_MODE   m_FillMode;			// current fill mode
	SizeType	m_winSize;			// client area dimensions
	PointType  	m_ptAnchor;			// anchor point for current operation
	PointType	m_ptLastMouse;		// last recorded mouse position
	RectType   	m_rctSel;			// current 2d selection
	RectType   	m_rctArea;			// current rectangle for area measurement

	bool mb_RenderDim;				// render outer dimensions
	bool mb_Selecting;				// select box is active
	bool mb_DebugInfo;				// display debug text

	int m_FrameBuffer;				// default frame buffer to render to

	std::vector<PointType> mv_Path;	// connected points defining path profile, pixels

	// lighting & colors

	float m_colorTEXT[4];

	float m_colorAXIS_X[4];
	float m_colorAXIS_Y[4];
	float m_colorAXIS_Z[4];

	// context menu actions

	QAction* mp_actionViewTop;			// top, ortho view
	QAction* mp_actionViewFront;
	QAction* mp_actionViewSide;
	QAction* mp_actionView3D;			// 3D perspective view

	QAction* mp_actionFillSolid;
	QAction* mp_actionFillWire;

	QAction* mp_actionRenderAxes;
	QAction* mp_actionRenderDim;
	QAction* mp_actionRenderLights;
	QAction* mp_actionRenderSkySphere;
	QAction* mp_actionRenderSkyBox;
	QAction* mp_actionRenderSkyNone;
	QAction* mp_actionRenderTerrain;
	QAction* mp_actionRenderTileInfo;

	QAction* mp_actionClear;
	QAction* mp_actionProperties;

	MainWindow* mp_Parent;
	ProfileWindow* mp_ProfileWindow;

private slots:

	void OnViewTop();
	void OnViewFront();
	void OnViewSide();
	void OnView3D();

	void OnFillSolid();
	void OnFillWire();

	void OnRenderAxes();
	void OnRenderDim();
	void OnRenderLights();
	void OnRenderSkySphere();
	void OnRenderSkyBox();
	void OnRenderSkyNone();
	void OnRenderTerrain();
	void OnRenderTileInfo();

	void OnClear();
	void OnProperties();

private:

	void RenderArea(QPainter* pPainter);
	void RenderDistance(QPainter* pPainter);
	void RenderDimensions();
	void RenderPath(QPainter* pPainter);
	void RenderSelectBox();
	void RenderText(QPainter* pPainter);

	// coordinate mapping
	void PixelToWorld(int xp, int yp, double& worldX, double& worldY);
	bool WorldToPixel(double worldX, double worldY, int &xp, int &yp);
	bool PixelToTerrainRowCol(int x, int y, int& row, int& col);

	const char* GetViewName();
	AXIS ViewToAxis(View viewId);
	void SetupMatrices();
	bool InitializeShaders();
	void Update3DCameraHeight();
	void UpdateOrthoScale();
	void PushText(int x, int y, const char* text);
	double GetMinViewHeight();
	void ShowPathProfile();

	void SetCurrentFile(const QString& fileName);
	QString StrippedName(const QString& fullFileName);

	void CreateActions();

	// terrain modification functions
	void Excavate(RectType rectPix);
	void Route(int startX, int startY, int endX, int endY);
	void TexPaint(int startX, int startY, int endX = -1, int endY = -1);
	void ChangeElevation(double delta, double scaleFactor, bool isDelta);
	void SmoothElevation();
};

#endif // #ifndef TERRAINWINDOW_H
