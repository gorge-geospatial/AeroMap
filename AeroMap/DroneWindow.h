#ifndef DRONEWINDOW_H
#define DRONEWINDOW_H

#include "AeroMap.h"	        // application header
#include "ShaderGL.h"
#include "VertexBufferGL.h"

#include <QOpenGLWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>

class MainWindow;

class DroneWindow : public QOpenGLWidget
{
    Q_OBJECT

public:
	
	DroneWindow(QWidget* parent);
	~DroneWindow();

	void ResetView();

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

	struct ImageType
	{
		XString file_name;
		VertexBufferGL* pVB;

		ImageType()
			: pVB(nullptr)
		{

		}
	};
	std::vector<ImageType> m_ImageList;

	double m_ThumbScale;
	int m_ThumbHeight;

	QFont m_Font;				// default font

	ShaderGL m_shaderImage;		// 2d image shader
	ShaderGL m_shaderSS;		// screen space shader

	mat4 m_matModel;				// current matrices
	mat4 m_matView;
	mat4 m_matProjection;

	SizeType	m_winSize;			// client area dimensions
	PointType  	m_ptAnchor;			// anchor point for current operation
	PointType	m_ptLastMouse;		// last recorded mouse position
	RectType   	m_rctSel;			// current 2d selection

	bool mb_Selecting;				// selecting vertices
	bool mb_DebugInfo;				// render add'l debug info

	float m_colorBACK[4];			// background color
	float m_colorTEXT[4];

	MainWindow* mp_Parent;

private:

	int LoadImageList();
	int GetImageCount();

	void RenderSelectBox();
	void RenderText(QPainter* pPainter);

	bool InitializeShaders();

	// lighting attributes
	void SetBackColor(UInt8 R, UInt8 G, UInt8 B);
	void GetBackColor(UInt8& R, UInt8& G, UInt8& B);

	void CreateActions();
};

#endif // #ifndef DRONEWINDOW_H
