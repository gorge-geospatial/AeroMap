#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "AeroMap.h"			// Application header

#include "OutputWindow.h"
#include "ProjectWindow.h"

// view windows
#include "DroneWindow.h"		// drone photogrammetry view
#include "LidarWindow.h"		// lidar view
#include "OrthoWindow.h"		// orthophoto view
#include "TerrainWindow.h"		// terrain model view

#include <QComboBox>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSignalMapper>

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow();
	~MainWindow();

	void ShowStatus(const char* status);
	void UpdateChildWindows();

	DroneWindow* GetDroneWindow() { return mp_DroneWindow; }
	LidarWindow* GetLidarWindow() { return mp_LidarWindow; }
	OrthoWindow* GetOrthoWindow() { return mp_OrthoWindow; }
	TerrainWindow* GetTerrainWindow() { return mp_TerrainWindow; }

public slots:

	void OnProjectChanged();
	void OnViewChanged(AeroMap::ViewType view, int subItem);

protected:

    void closeEvent(QCloseEvent* event) override;
	void paintEvent(QPaintEvent *event) override;

private slots:

	void OnFileNew();
	void OnFileOpen();
	void OnFileOpenRecent();
	void OnFileSave();
	void OnFileSaveAs();
	void OnFileClear();

	void OnViewProject();
	void OnViewOutput();
	void OnViewMenuShow();

	//void OnReindex();
	//void OnReindexStart();
	//void OnFileIndexed(QString fileName);
	//void OnReindexFinished(unsigned int fileCount);

	void OnToolNone();
	void OnToolSelect();
	void OnToolDistance();
	void OnToolArea();
	void OnToolLight();
	void OnToolContour();
	void OnToolElev();
	void OnToolProfile();

	void OnToolViewHome();
	void OnToolViewZoom();
	void OnToolViewRotate();
	void OnToolViewPan();
	void OnToolViewZoomIn();
	void OnToolViewZoomOut();
	void OnToolExport();

	void OnColorIndexChanged(int index);

	void OnDroneProc();
	void OnLidarDxm();

	void OnScaleColor();
	void OnConfig();

	void OnHelpAbout();

    void UpdateMenus();
    void SetActiveSubWindow(QWidget* window);

private:

	void CloseChildWindows();

    void CreateActions();
    void CreateMenus();
    void CreateToolBars();
    void CreateStatusBar();
	void CreateColorScaleCombo();

	void LoadProject(const char* pathName);
	void SetCurrentFile(const QString& fileName);
    void UpdateRecentFileActions();
	void UpdateWindowTitle();
    QString StrippedName(const QString& fullFileName);

    void ReadSettings();
    void WriteSettings();

private:

    QMdiArea* mp_mdiArea;
    QSignalMapper* mp_windowMapper;

	// mdi subwindows
	DroneWindow* mp_DroneWindow;
	LidarWindow* mp_LidarWindow;
	OrthoWindow* mp_OrthoWindow;
	TerrainWindow* mp_TerrainWindow;

	ProjectWindow* mp_ProjectWindow;
	OutputWindow* mp_OutputWindow;

	//GeoIndex* mp_GeoIndex;
	
	bool mb_PaintMask;

	QMenu* mp_menuFile;
	QMenu* mp_menuView;
	QMenu* mp_menuSetup;
	QMenu* mp_menuHelp;

	QToolBar* mp_toolBarFile;
	QToolBar* mp_toolBarDrone;			// drone photogrammetry toolbar
	QToolBar* mp_toolBarLidar;			// lidar workspace toolbar
	QToolBar* mp_toolBarOrtho;			// orthophoto view toolbar
	QToolBar* mp_toolBarTerrain;		// terrain view toolbar

	QAction* mp_actionFileNew;
	QAction* mp_actionFileOpen;
	QAction* mp_actionFileSave;
	QAction* mp_actionFileSaveAs;
	QAction* mp_actionFileExport;
	QAction* mp_actionFileExit;
	QAction* mp_actionFileClear;

	QAction* mp_actionDataReindex;

	QAction* mp_actionSetupScaleColor;
	QAction* mp_actionSetupConfig;

	QAction* mp_actionViewProject;
	QAction* mp_actionViewOutput;

	QAction* mp_actionHelpAbout;

	QAction* mp_actionDroneProc;			// drone image processing
	QAction* mp_actionLidarDxm;				// lidar tools

	QAction* mp_actionToolNone;
	QAction* mp_actionToolSelect;
	QAction* mp_actionToolLight;
	QAction* mp_actionToolElev;
	QAction* mp_actionToolContour;
	QAction* mp_actionToolProfile;

	QAction* mp_actionToolViewHome;
	QAction* mp_actionToolViewScale;
	QAction* mp_actionToolViewRotate;
	QAction* mp_actionToolViewPan;
	QAction* mp_actionToolViewZoomIn;
	QAction* mp_actionToolViewZoomOut;
	QAction* mp_actionToolExport;

	QAction* mp_actionToolDistance;			// measure distance
	QAction* mp_actionToolArea;				// measure area

	QAction* mp_actionSep;

	QComboBox* mp_cboColorTerrain;		// terrain color scale
	QComboBox* mp_cboAttrLidar;			// property used for color (return #, z value, etc.)
	QComboBox* mp_cboColorLidar;		// color scale combo box, lidar

	enum { MAX_RECENT_FILES = 4 };
    QAction* mp_recentFileActs[MAX_RECENT_FILES];
};

#endif // #ifndef MAINWINDOW_H
