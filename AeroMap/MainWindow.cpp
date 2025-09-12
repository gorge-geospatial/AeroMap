// MainWindow.cpp
//

#include "MarkLib.h"				// perftimer, etc.
#include "TextFile.h"
#include "AboutDlg.h"				// Help about dialog
#include "ConfigDlg.h"				// Edit configuration dialog
#include "ScaleColorDlg.h"			// Edit color scales
#include "LidarModel.h"				// Lidar DTM/DSM
#include "LidarModelDlg.h"			// Lidar DTM/DSM options
#include "DroneProc.h"				// Drone photogrammetry pipeline
#include "DroneProcDlg.h"			// Drone photogrammetry options

#include "MainWindow.h"

#include <QtGui>
#include <QDockWidget>
#include <QFileDialog>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

MainWindow::MainWindow()
	: mp_DroneWindow(nullptr)
	, mp_LidarWindow(nullptr)
	, mp_OrthoWindow(nullptr)
	, mp_TerrainWindow(nullptr)
	, mp_ProjectWindow(nullptr)
	, mp_OutputWindow(nullptr)
	, mp_menuSetup(nullptr)
	, mp_menuFile(nullptr)
	, mp_menuView(nullptr)
	, mp_menuHelp(nullptr)
	, mp_toolBarFile(nullptr)
	, mp_toolBarDrone(nullptr)
	, mp_toolBarLidar(nullptr)
	, mp_toolBarOrtho(nullptr)
	, mp_toolBarTerrain(nullptr)
	, mp_actionFileNew(nullptr)
	, mp_actionFileOpen(nullptr)
	, mp_actionFileSave(nullptr)
	, mp_actionFileSaveAs(nullptr)
	, mp_actionFileExport(nullptr)
	, mp_actionFileExit(nullptr)
	, mp_actionViewProject(nullptr)
	, mp_actionViewOutput(nullptr)
	, mp_actionHelpAbout(nullptr)
	, mp_actionDataReindex(nullptr)
	, mp_actionSetupConfig(nullptr)
	, mp_actionSetupScaleColor(nullptr)
	, mp_actionToolNone(nullptr)
	, mp_actionToolSelect(nullptr)
	, mp_actionToolElev(nullptr)
	, mp_actionToolContour(nullptr)
	, mp_actionToolProfile(nullptr)
	, mp_actionDroneProc(nullptr)
	, mp_actionLidarDxm(nullptr)
	, mp_actionToolDistance(nullptr)
	, mp_actionToolArea(nullptr)
	, mp_actionToolLight(nullptr)
	, mp_actionToolViewHome(nullptr)
	, mp_actionToolViewScale(nullptr)
	, mp_actionToolViewRotate(nullptr)
	, mp_actionToolViewPan(nullptr)
	, mp_actionToolViewZoomIn(nullptr)
	, mp_actionToolViewZoomOut(nullptr)
	, mp_actionToolExport(nullptr)
	//, mp_GeoIndex(nullptr)
	, mp_cboAttrLidar(nullptr)
	, mp_cboColorLidar(nullptr)
	, mp_cboColorTerrain(nullptr)
	, mb_PaintMask(false)
{
    mp_mdiArea = new QMdiArea();
    mp_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mp_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mp_mdiArea->setViewMode(QMdiArea::ViewMode::SubWindowView);
    setCentralWidget(mp_mdiArea);
	verify_connect(mp_mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(UpdateMenus()));
    mp_windowMapper = new QSignalMapper(this);
	verify_connect(mp_windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(SetActiveSubWindow(QWidget*)));

	CreateColorScaleCombo();

	CreateActions();
    CreateMenus();
    CreateToolBars();
    CreateStatusBar();
	UpdateRecentFileActions();
    UpdateMenus();

    ReadSettings();

	QDockWidget* pDockProject = new QDockWidget(tr("Project"), this);
	pDockProject->setAllowedAreas(Qt::LeftDockWidgetArea);				// can only dock on left
	mp_ProjectWindow = new ProjectWindow(pDockProject);					// create widget with dock as parent
	pDockProject->setWidget(mp_ProjectWindow);							// then set the widget (seems redundant)
	addDockWidget(Qt::LeftDockWidgetArea, pDockProject);				// add it to the main window

	QDockWidget* pDockOutput = new QDockWidget(tr("Output"), this);
	pDockOutput->setAllowedAreas(Qt::BottomDockWidgetArea);				// can only dock on bottom
	mp_OutputWindow = new OutputWindow(pDockOutput);					// create widget with dock as parent
	pDockOutput->setWidget(mp_OutputWindow);							// then set the widget (seems redundant)
	addDockWidget(Qt::BottomDockWidgetArea, pDockOutput);				// add it to the main window
	resizeDocks({ pDockOutput }, { 50 }, Qt::Horizontal);				// as of 5.6, this method will set the size of dock widgets, in pixels
	GetApp()->SetOutputWindow(mp_OutputWindow);							// let app know about it

	setWindowTitle(tr(APP_NAME));
    setUnifiedTitleAndToolBarOnMac(true);

	// signals & slots
	verify_connect(GetApp(), SIGNAL(view_change_sig(AeroMap::ViewType, int)), this, SLOT(OnViewChanged(AeroMap::ViewType, int)));
	verify_connect(&GetProject(), SIGNAL(projectChanged_sig()), this, SLOT(OnProjectChanged()));

	// load last loaded project
	QSettings settings(ORG_KEY, APP_KEY);
	XString lastFile = settings.value(LAST_FILE_KEY).toString();
	if (lastFile.GetLength() > 0)
		LoadProject(lastFile.c_str());
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (GetApp()->GetProject().IsDirty() == true)
	{
		QMessageBox::StandardButton ret = QMessageBox::warning(
			this,
			tr("Save Project File"),
			tr("Project modified, save?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		switch (ret) {
		case QMessageBox::Yes:
			GetApp()->GetProject().Save();
			break;
		case QMessageBox::No:
			// continue closing
			break;
		case QMessageBox::Cancel:
			event->ignore();
			return;
			break;
		}
	}

	if (mp_ProjectWindow != nullptr)
		mp_ProjectWindow->SaveState();

	delete mp_cboAttrLidar;
	delete mp_cboColorLidar;
	delete mp_cboColorTerrain;

	//remove all windows first?
	//mp_mdiArea->removeSubWindow

	delete mp_DroneWindow;
	delete mp_LidarWindow;
	delete mp_OrthoWindow;
	delete mp_TerrainWindow;

	delete mp_ProjectWindow;
	delete mp_OutputWindow;

	//delete mp_GeoIndex;

	delete mp_menuFile;
	delete mp_menuView;
	delete mp_menuSetup;
	delete mp_menuHelp;

	delete mp_actionFileNew;
	delete mp_actionFileOpen;
	delete mp_actionFileSave;
	delete mp_actionFileSaveAs;
	delete mp_actionFileExport;
	delete mp_actionFileExit;
	delete mp_actionViewProject;
	delete mp_actionViewOutput;
	delete mp_actionSetupConfig;
	delete mp_actionSetupScaleColor;
	delete mp_actionHelpAbout;

	delete mp_actionDataReindex;

	delete mp_actionToolNone;
	delete mp_actionToolSelect;
	delete mp_actionToolElev;
	delete mp_actionToolProfile;
	delete mp_actionToolContour;
	delete mp_actionToolDistance;
	delete mp_actionToolArea;
	delete mp_actionToolLight;
	delete mp_actionDroneProc;
	delete mp_actionLidarDxm;
	delete mp_actionToolViewHome;
	delete mp_actionToolViewScale;
	delete mp_actionToolViewRotate;
	delete mp_actionToolViewPan;
	delete mp_actionToolViewZoomIn;
	delete mp_actionToolViewZoomOut;
	delete mp_actionToolExport;
		
	delete mp_toolBarFile;
//TODO:
	//can't delete this until i figure out how to remove widgets
	//delete mp_toolBarDrone;
	//delete mp_toolBarLidar;
	//delete mp_toolBarOrtho;
	//delete mp_toolBarTerrain;
	delete mp_mdiArea;
	delete mp_windowMapper;

	WriteSettings();
	event->accept();
}

void MainWindow::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
}

void MainWindow::CreateActions()
{
	// file actions

	mp_actionFileNew = new QAction(QIcon(":/Images/new.png"), tr("New Project"), this);
	mp_actionFileNew->setShortcuts(QKeySequence::New);
	mp_actionFileNew->setStatusTip(tr("Create new AeroMap project"));
	verify_connect(mp_actionFileNew, SIGNAL(triggered()), this, SLOT(OnFileNew()));

	mp_actionFileOpen = new QAction(QIcon(":/Images/open.png"), tr("&Open"), this);
	mp_actionFileOpen->setShortcuts(QKeySequence::Open);
	mp_actionFileOpen->setStatusTip(tr("Open a AeroMap project"));
	verify_connect(mp_actionFileOpen, SIGNAL(triggered()), this, SLOT(OnFileOpen()));

	mp_actionFileSave = new QAction(QIcon(":/Images/save.png"), tr("&Save Project"), this);
	mp_actionFileSave->setShortcuts(QKeySequence::Save);
	mp_actionFileSave->setStatusTip(tr("Save project file"));
	verify_connect(mp_actionFileSave, SIGNAL(triggered()), this, SLOT(OnFileSave()));

	mp_actionFileSaveAs = new QAction(QIcon(":/Images/save.png"), tr("&Save Project As ..."), this);
	mp_actionFileSaveAs->setShortcuts(QKeySequence::SaveAs);
	mp_actionFileSaveAs->setStatusTip(tr("Save a copy of project file"));
	verify_connect(mp_actionFileSaveAs, SIGNAL(triggered()), this, SLOT(OnFileSaveAs()));

	for (int i = 0; i < MAX_RECENT_FILES; ++i)
	{
		mp_recentFileActs[i] = new QAction(this);
		mp_recentFileActs[i]->setVisible(false);
		connect(mp_recentFileActs[i], SIGNAL(triggered()), this, SLOT(OnFileOpenRecent()));
	}

	mp_actionFileClear = new QAction(QIcon(""), tr("Clear History"), this);
	mp_actionFileClear->setStatusTip(tr("Clear MRU list"));
	verify_connect(mp_actionFileClear, SIGNAL(triggered()), this, SLOT(OnFileClear()));

	mp_actionFileExit = new QAction(tr("E&xit"), this);
	mp_actionFileExit->setShortcuts(QKeySequence::Quit);
	mp_actionFileExit->setStatusTip(tr("Exit the application"));
	verify_connect(mp_actionFileExit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

	// setup actions

	mp_actionSetupScaleColor = new QAction(QIcon(""), tr("Color Scales"), this);
	mp_actionSetupScaleColor->setStatusTip(tr("Create and modify color scales"));
	verify_connect(mp_actionSetupScaleColor, SIGNAL(triggered()), this, SLOT(OnScaleColor()));

	mp_actionSetupConfig = new QAction(QIcon(""), tr("Configuration"), this);
	mp_actionSetupConfig->setStatusTip(tr("Edit system configuration"));
	verify_connect(mp_actionSetupConfig, SIGNAL(triggered()), this, SLOT(OnConfig()));

	mp_actionSep = new QAction(this);
	mp_actionSep->setSeparator(true);

	// view actions

	mp_actionViewProject = new QAction(QIcon(""), tr("Project Window"), this);
	mp_actionViewProject->setStatusTip(tr("Show/hide the project window"));
	mp_actionViewProject->setCheckable(true);
	verify_connect(mp_actionViewProject, SIGNAL(triggered()), this, SLOT(OnViewProject()));

	mp_actionViewOutput = new QAction(QIcon(""), tr("Output Window"), this);
	mp_actionViewOutput->setStatusTip(tr("Show/hide the output window"));
	mp_actionViewOutput->setCheckable(true);
	verify_connect(mp_actionViewOutput, SIGNAL(triggered()), this, SLOT(OnViewOutput()));

	// help actions
	 
	mp_actionHelpAbout = new QAction(QIcon(""), tr("About ..."), this);
	mp_actionHelpAbout->setStatusTip(tr("Show Help About screen"));
	verify_connect(mp_actionHelpAbout, SIGNAL(triggered()), this, SLOT(OnHelpAbout()));

	// toolbar actions

	mp_actionToolNone = new QAction(QIcon(":/Images/arrow32.png"), tr("No Tool"), this);
	mp_actionToolNone->setStatusTip(tr("Clear tool selection"));
	verify_connect(mp_actionToolNone, SIGNAL(triggered()), this, SLOT(OnToolNone()));

	mp_actionToolViewHome = new QAction(QIcon(":/Images/home32.png"), tr("Home"), this);
	mp_actionToolViewHome->setStatusTip(tr("Return to home view"));
	verify_connect(mp_actionToolViewHome, SIGNAL(triggered()), this, SLOT(OnToolViewHome()));

	mp_actionToolSelect = new QAction(QIcon(":/Images/select32.png"), tr("Select"), this);
	mp_actionToolSelect->setStatusTip(tr("Select vertices"));
	verify_connect(mp_actionToolSelect, SIGNAL(triggered()), this, SLOT(OnToolSelect()));

	mp_actionToolViewScale = new QAction(QIcon(":/Images/zoomview32.png"), tr("Zoom View"), this);
	mp_actionToolViewScale->setStatusTip(tr("Move camera in/out"));
	verify_connect(mp_actionToolViewScale, SIGNAL(triggered()), this, SLOT(OnToolViewZoom()));

	mp_actionToolViewRotate = new QAction(QIcon(":/Images/rotateview32.png"), tr("Rotate View"), this);
	mp_actionToolViewRotate->setStatusTip(tr("Rotate camera"));
	verify_connect(mp_actionToolViewRotate, SIGNAL(triggered()), this, SLOT(OnToolViewRotate()));

	mp_actionToolViewPan = new QAction(QIcon(":/Images/panview32.png"), tr("Pan View"), this);
	mp_actionToolViewPan->setStatusTip(tr("Pan camera"));
	verify_connect(mp_actionToolViewPan, SIGNAL(triggered()), this, SLOT(OnToolViewPan()));

	mp_actionToolViewZoomIn = new QAction(QIcon(":/Images/zoomin32.png"), tr("Zoom In"), this);
	mp_actionToolViewZoomIn->setStatusTip(tr("Zoom in"));
	verify_connect(mp_actionToolViewZoomIn, SIGNAL(triggered()), this, SLOT(OnToolViewZoomIn()));

	mp_actionToolViewZoomOut = new QAction(QIcon(":/Images/zoomout32.png"), tr("Zoom Out"), this);
	mp_actionToolViewZoomOut->setStatusTip(tr("Zoom out"));
	verify_connect(mp_actionToolViewZoomOut, SIGNAL(triggered()), this, SLOT(OnToolViewZoomOut()));

	mp_actionToolExport = new QAction(QIcon(":/Images/export32.png"), tr("Export"), this);
	mp_actionToolExport->setStatusTip(tr("Export"));
	verify_connect(mp_actionToolExport, SIGNAL(triggered()), this, SLOT(OnToolExport()));

	mp_actionToolLight = new QAction(QIcon(":/Images/light.png"), tr("Lights"), this);
	mp_actionToolLight->setStatusTip(tr("Move lights"));
	verify_connect(mp_actionToolLight, SIGNAL(triggered()), this, SLOT(OnToolLight()));

	mp_actionToolDistance = new QAction(QIcon(":/Images/meas-distance32.png"), tr("Measure Distance"), this);
	mp_actionToolDistance->setStatusTip(tr("Measure linear distance"));
	verify_connect(mp_actionToolDistance, SIGNAL(triggered()), this, SLOT(OnToolDistance()));

	mp_actionToolArea = new QAction(QIcon(":/Images/meas-area32.png"), tr("Measure Area"), this);
	mp_actionToolArea->setStatusTip(tr("Measure area"));
	verify_connect(mp_actionToolArea, SIGNAL(triggered()), this, SLOT(OnToolArea()));

	mp_actionToolElev = new QAction(QIcon(":/Images/elev32.png"), tr("Elevation"), this);
	mp_actionToolElev->setStatusTip(tr("Modify elevation"));
	verify_connect(mp_actionToolElev, SIGNAL(triggered()), this, SLOT(OnToolElev()));

	mp_actionToolContour = new QAction(QIcon(":/Images/contour32.png"), tr("Contour"), this);
	mp_actionToolContour->setStatusTip(tr("Modify elevation"));
	verify_connect(mp_actionToolContour, SIGNAL(triggered()), this, SLOT(OnToolContour()));

	mp_actionToolProfile = new QAction(QIcon(":/Images/profile32.png"), tr("Profile"), this);
	mp_actionToolProfile->setStatusTip(tr("Modify elevation"));
	verify_connect(mp_actionToolProfile, SIGNAL(triggered()), this, SLOT(OnToolProfile()));

	mp_actionDroneProc = new QAction(QIcon(":/Images/drone-proc32.png"), tr("Photogrammetry"), this);
	mp_actionDroneProc->setStatusTip(tr("Process drone imagery"));
	verify_connect(mp_actionDroneProc, SIGNAL(triggered()), this, SLOT(OnDroneProc()));

	mp_actionLidarDxm = new QAction(QIcon(":/Images/dxm32.png"), tr("DTM/DSM"), this);
	mp_actionLidarDxm->setStatusTip(tr("Create DTM/DSM from point cloud"));
	verify_connect(mp_actionLidarDxm, SIGNAL(triggered()), this, SLOT(OnLidarDxm()));

	mp_actionFileExport = new QAction(QIcon(""), tr("&Export"), this);
	mp_actionFileExport->setStatusTip(tr("Export data"));
	verify_connect(mp_actionFileExport, SIGNAL(triggered()), this, SLOT(OnToolExport()));
}

void MainWindow::CreateMenus()
{
	mp_menuFile = menuBar()->addMenu(tr("&File"));
	mp_menuFile->addAction(mp_actionFileNew);
	mp_menuFile->addAction(mp_actionFileOpen);
	mp_menuFile->addAction(mp_actionFileSave);
	mp_menuFile->addAction(mp_actionFileSaveAs);
	mp_menuFile->addAction(mp_actionFileExport);
	mp_actionSep = mp_menuFile->addSeparator();
	for (int i = 0; i < MAX_RECENT_FILES; ++i)
		mp_menuFile->addAction(mp_recentFileActs[i]);
	mp_menuFile->addSeparator();
	mp_menuFile->addAction(mp_actionFileClear);
	mp_menuFile->addSeparator();
	mp_menuFile->addAction(mp_actionFileExit);

	mp_menuView = menuBar()->addMenu(tr("&View"));
	mp_menuView->addAction(mp_actionViewProject);
	mp_menuView->addAction(mp_actionViewOutput);

	mp_menuSetup = menuBar()->addMenu(tr("&Setup"));
	mp_menuSetup->addAction(mp_actionSetupScaleColor);
	mp_menuSetup->addAction(mp_actionSetupConfig);

	mp_menuHelp = menuBar()->addMenu(tr("&Help"));
	mp_menuHelp->addAction(mp_actionHelpAbout);
	
	verify_connect(mp_menuView, SIGNAL(aboutToShow()), this, SLOT(OnViewMenuShow()));
}

void MainWindow::CreateToolBars()
{
	mp_toolBarFile = addToolBar(tr("File"));
	mp_toolBarFile->addAction(mp_actionFileNew);
	mp_toolBarFile->addAction(mp_actionFileOpen);
	mp_toolBarFile->addAction(mp_actionFileSave);
	mp_toolBarFile->show();

	mp_toolBarDrone = addToolBar(tr("Photogrammetry Tools"));
	mp_toolBarDrone->hide();
	mp_toolBarDrone->addAction(mp_actionToolNone);
	mp_toolBarDrone->addAction(mp_actionToolViewHome);
	mp_toolBarDrone->addSeparator();
	mp_toolBarDrone->addAction(mp_actionDroneProc);

	mp_toolBarLidar = addToolBar(tr("Lidar Tools"));
	mp_toolBarLidar->hide();
	mp_toolBarLidar->addAction(mp_actionToolNone);
	mp_toolBarLidar->addAction(mp_actionLidarDxm);
	mp_toolBarLidar->addSeparator();
	mp_toolBarLidar->addWidget(new QLabel("Color Scale: "));
	mp_toolBarLidar->addWidget(mp_cboColorLidar);
	mp_toolBarLidar->addWidget(new QLabel(" Attribute: "));
	mp_toolBarLidar->addWidget(mp_cboAttrLidar);

	mp_toolBarOrtho = addToolBar(tr("Orthophoto Tools"));
	mp_toolBarOrtho->hide();
	mp_toolBarOrtho->addAction(mp_actionToolNone);
	mp_toolBarOrtho->addAction(mp_actionToolViewHome);
	mp_toolBarOrtho->addAction(mp_actionToolDistance);
	mp_toolBarOrtho->addAction(mp_actionToolArea);

	mp_toolBarTerrain = addToolBar(tr("Terrain Tools"));
	mp_toolBarTerrain->hide();
	mp_toolBarTerrain->addAction(mp_actionToolNone);
	mp_toolBarTerrain->addSeparator();
	mp_toolBarTerrain->addAction(mp_actionToolViewHome);
	mp_toolBarTerrain->addAction(mp_actionToolSelect);
	mp_toolBarTerrain->addAction(mp_actionToolViewPan);
	mp_toolBarTerrain->addSeparator();
	mp_toolBarTerrain->addAction(mp_actionToolContour);
	mp_toolBarTerrain->addAction(mp_actionToolElev);
	mp_toolBarTerrain->addSeparator();
	mp_toolBarTerrain->addAction(mp_actionToolDistance);
	mp_toolBarTerrain->addAction(mp_actionToolArea);
	mp_toolBarTerrain->addAction(mp_actionToolProfile);
	mp_toolBarTerrain->addSeparator();
	mp_toolBarTerrain->addWidget(new QLabel("Color Scale: "));
	mp_toolBarTerrain->addWidget(mp_cboColorTerrain);

	mp_toolBarFile->show();
	mp_toolBarDrone->hide();
	mp_toolBarLidar->hide();
	mp_toolBarOrtho->hide();
	mp_toolBarTerrain->hide();
}

void MainWindow::CreateStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void MainWindow::CreateColorScaleCombo()
{
	// Load toolbar color scale combo boxes.
	//
	// All scales assumed to be in app data path.
	//

	delete mp_cboAttrLidar;
	delete mp_cboColorLidar;
	delete mp_cboColorTerrain;

	mp_cboAttrLidar = new QComboBox();
	mp_cboColorLidar = new QComboBox();
	mp_cboColorTerrain = new QComboBox();

	mp_cboColorTerrain->addItem("");
	mp_cboColorTerrain->addItem(ORTHO_KEY);

	// load all defined color scales

	QDir dir(GetApp()->GetAppDataPath().c_str());
	dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);

	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i)
	{
		QFileInfo fileInfo = list.at(i);
		if (fileInfo.isFile())
		{
			XString fileName = fileInfo.fileName().toLatin1().constData();
			XString filePath = fileInfo.absoluteFilePath().toLatin1().constData();
			if (fileName.EndsWithNoCase(ScaleColor::GetDefaultExt()))
			{
				mp_cboColorLidar->addItem(fileName.c_str());
				mp_cboColorTerrain->addItem(fileName.c_str());
			}
		}
	}

	// load lidar properties than can have color applied
	mp_cboAttrLidar->addItem("Z Value", QVariant((int)LidarWindow::LidarAttr::Z));
	mp_cboAttrLidar->addItem("Return Number", QVariant((int)LidarWindow::LidarAttr::Return));
	mp_cboAttrLidar->addItem("Classification", QVariant((int)LidarWindow::LidarAttr::Class));

	verify_connect(mp_cboColorTerrain, SIGNAL(currentIndexChanged(int)), this, SLOT(OnColorIndexChanged(int)));
	verify_connect(mp_cboColorLidar, SIGNAL(currentIndexChanged(int)), this, SLOT(OnColorIndexChanged(int)));
	verify_connect(mp_cboAttrLidar, SIGNAL(currentIndexChanged(int)), this, SLOT(OnColorIndexChanged(int)));
}

void MainWindow::LoadProject(const char* pathName)
{
	// Load AeroMap project.
	//

	GetApp()->LogWrite("Loading %s ...", pathName);

	CloseChildWindows();

	GetProject().Load(pathName);
	SetCurrentFile(pathName);

	// show photogrammetry view by default
	OnViewChanged(AeroMap::ViewType::Drone, 0);

	GetApp()->LogWrite("%s loaded", pathName);
}

void MainWindow::SetCurrentFile(const QString& fileName)
{
	// Set the current / mru project file.
	//

	if (fileName.length() == 0)
		return;

	QSettings settings(ORG_KEY, APP_KEY);
	QStringList files = settings.value("recentFileList").toStringList();
	files.removeAll(fileName);
	files.prepend(fileName);
	while (files.size() > MAX_RECENT_FILES)
		files.removeLast();

	settings.setValue("recentFileList", files);

	UpdateRecentFileActions();
	UpdateWindowTitle();

	// save the last loaded project
	settings.setValue(LAST_FILE_KEY, fileName);
}

void MainWindow::UpdateRecentFileActions()
{
	QSettings settings(ORG_KEY, APP_KEY);
	QStringList files = settings.value("recentFileList").toStringList();

	int numRecentFiles = qMin(files.size(), (int)MAX_RECENT_FILES);

	for (int i = 0; i < numRecentFiles; ++i)
	{
		QString text = tr("&%1 %2").arg(i + 1).arg(StrippedName(files[i]));
		mp_recentFileActs[i]->setText(text);
		mp_recentFileActs[i]->setData(files[i]);
		mp_recentFileActs[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MAX_RECENT_FILES; ++j)
		mp_recentFileActs[j]->setVisible(false);

	mp_actionSep->setVisible(numRecentFiles > 0);
}

void MainWindow::UpdateWindowTitle()
{
	XString str = APP_NAME;
	if (GetProject().GetFileName().IsEmpty() == false)
	{
		str += " [";
		str += GetProject().GetFileName();
		str += "]";

		if (GetProject().IsDirty())
			str += " *";
	}
	setWindowTitle(str.c_str());
}

QString MainWindow::StrippedName(const QString& fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void MainWindow::ShowStatus(const char* status)
{
	QStatusBar* pStatusBar = statusBar();
	if (pStatusBar)
	{
		pStatusBar->showMessage(status);
	}
}

void MainWindow::CloseChildWindows()
{
	// Close all mdi child windows.
	//

	mp_mdiArea->closeAllSubWindows();

	if (mp_DroneWindow)
	{
		mp_mdiArea->removeSubWindow(mp_DroneWindow);
		delete mp_DroneWindow;
		mp_DroneWindow = nullptr;
	}
	if (mp_LidarWindow)
	{
		mp_mdiArea->removeSubWindow(mp_LidarWindow);
		delete mp_LidarWindow;
		mp_LidarWindow = nullptr;
	}
	if (mp_OrthoWindow)
	{
		mp_mdiArea->removeSubWindow(mp_OrthoWindow);
		delete mp_OrthoWindow;
		mp_OrthoWindow = nullptr;
	}
	if (mp_TerrainWindow)
	{
		mp_mdiArea->removeSubWindow(mp_TerrainWindow);
		delete mp_TerrainWindow;
		mp_TerrainWindow = nullptr;
	}
}

void MainWindow::UpdateChildWindows()
{
	if (mp_DroneWindow != nullptr)
		mp_DroneWindow->update();
	if (mp_LidarWindow != nullptr)
		mp_LidarWindow->update();
	if (mp_OrthoWindow != nullptr)
		mp_OrthoWindow->update();
	if (mp_DroneWindow != nullptr)
		mp_DroneWindow->update();
}

void MainWindow::UpdateMenus()
{
}

void MainWindow::ReadSettings()
{
	QSettings settings(ORG_KEY, APP_KEY);
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
	QSize size = settings.value("size", QSize(400, 400)).toSize();
	move(pos);
	resize(size);
}

void MainWindow::WriteSettings()
{
	QSettings settings(ORG_KEY, APP_KEY);
	settings.setValue("pos", pos());
	settings.setValue("size", size());
}

void MainWindow::SetActiveSubWindow(QWidget* window)
{
	if (!window)
		return;
	mp_mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::OnFileNew()
{
	// Create new project.
	//

	CloseChildWindows();
	GetProject().Clear();
}

void MainWindow::OnFileOpen()
{
	// Open project.
	//

	char filter[255] = { 0 };
	sprintf(filter, "Project Files (*.%s)", Project::GetDefaultExt());

	XString defaultPath = GetApp()->GetAppDataPath().c_str();
	XString fileName = QFileDialog::getOpenFileName(
		this,
		tr("Open Project File"),
		defaultPath.c_str(),
		filter);

	if (fileName.GetLength() > 0)
	{
		LoadProject(fileName);
	}
}

void MainWindow::OnFileOpenRecent()
{
	// Open recent project.
	//

	QAction* action = qobject_cast<QAction *>(sender());
	if (action)
	{
		XString fileName = action->data().toString();
		if (fileName.GetLength() > 0)
		{
			if (AeroLib::FileExists(fileName) == false)
				QMessageBox::information(this, tr("Load Project"), "Project file not found", QMessageBox::StandardButton::Ok);
			else
				LoadProject(fileName.c_str());
		}
	}
}

void MainWindow::OnFileSave()
{
	// Write project file to disk.
	//

	XString fileName = GetProject().GetFileName();
	if (fileName.IsEmpty() == false)
	{
		GetProject().Save(fileName.c_str());
		UpdateWindowTitle();
	}
	else
	{
		OnFileSaveAs();
	}
}

void MainWindow::OnFileSaveAs()
{
	// Save a current project with new name.
	//

	XString defaultPath = GetProject().GetFileName().GetPathName();
	XString filter = XString::Format("AeroMap Project Files (*.%s)", GetProject().GetDefaultExt());

	XString fileName = QFileDialog::getSaveFileName(
		this,
		tr("Select Project File Name"),
		defaultPath.c_str(),
		filter.c_str());

	if (fileName.GetLength() > 0)
	{
		bool save = true;
		if (QFile::exists(fileName.c_str()) == true)
		{
			QMessageBox::StandardButton ret = QMessageBox::warning(
				this, 
				tr("Save Project File"), 
				tr("Project file exists, overwrite?"), 
				QMessageBox::Yes | QMessageBox::No);
				if (ret != QMessageBox::Yes)
					save = false;
		}
		if (save)
		{
			GetProject().Save(fileName.c_str());
			UpdateWindowTitle();
		}
	}
}

void MainWindow::OnViewProject()
{
	if (mp_ProjectWindow->isVisible())
		mp_ProjectWindow->hide();
	else
		mp_ProjectWindow->show();
}

void MainWindow::OnViewOutput()
{
	if (mp_OutputWindow->isVisible())
		mp_OutputWindow->hide();
	else
		mp_OutputWindow->show();
}

void MainWindow::OnViewMenuShow()
{
	mp_actionViewProject->setChecked(mp_ProjectWindow->isVisible());
	mp_actionViewOutput->setChecked(mp_OutputWindow->isVisible());
}

// running a function in another thread using QThread - without subclassing QThread

//void MainWindow::OnReindex()
//{
//	QMessageBox::StandardButton ret = QMessageBox::warning(
//		this, 
//		"Rebuild Geospatial Index", 
//		"Rebuild index? This can be time-consuming and must run to completion.", 
//		QMessageBox::Yes | QMessageBox::No);
//	if (ret != QMessageBox::Yes)
//		return;
//
//	delete mp_GeoIndex;
//	mp_GeoIndex = new GeoIndex(GetApp()->GetGeoDataPath());
//	QThread* pThread = new QThread();
//
//	mp_GeoIndex->moveToThread(pThread);
//
//	// starts the geoindex object after the thread has started; this method is run on the thread
//	verify_connect(pThread, SIGNAL(started()), this, SLOT(OnReindexStart()));
//	// links the geoindex object to the GUI
//	verify_connect(mp_GeoIndex, SIGNAL(fileIndexed(QString)), this, SLOT(OnFileIndexed(QString)));
//	// causes the thread's event loop to exit once the geoindex has finished
//	verify_connect(mp_GeoIndex, SIGNAL(finished(unsigned int)), this, SLOT(OnReindexFinished(unsigned int)));
//	// last 2 connects handle cleanup, since our objects were created without parents. the 'deleteLater' method causes 
//	// the objects to be deleted once there are no more pending events in the event loop. geoindex is deleted on the 
//	// thread, and the thread itself is actually deleted on the main thread. This 'deleteLater' call works for 
//	// geoindex, even though the thread's event loop would have already exited, because QThread does one final 
//	// loop to handle all deferred deletetion events once its event loop has exited. QApplication does the same.
//	//verify_connect(mp_GeoIndex, SIGNAL(finished(unsigned int)), mp_GeoIndex, SLOT(deleteLater()));
//	//verify_connect(mp_GeoIndex, SIGNAL(finished(unsigned int)), thread, SLOT(deleteLater()));
//
//	pThread->start();
//}
//
//void MainWindow::OnReindexStart()
//{
////may not be using this slot properly?
//	mp_GeoIndex->RebuildIndex();
//}
//
//void MainWindow::OnFileIndexed(QString fileName)
//{
//	mp_OutputWindow->AppendText(XString(fileName).c_str());
//}
//
//void MainWindow::OnReindexFinished(unsigned int fileCount)
//{
//	XString strMessage = XString::Format("Reindex complete - %u files indexed.", fileCount);
//	QMessageBox::information(this, tr("Reindex"), strMessage.c_str(), QMessageBox::StandardButton::Ok);
//}

void MainWindow::OnScaleColor()
{
	ScaleColorDlg dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
	}
}

void MainWindow::OnConfig()
{
	ConfigDlg dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
	}
}

void MainWindow::OnHelpAbout()
{
	// Show Help/About screen.
	//

	AboutDlg dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
	}
}

void MainWindow::OnToolNone()
{
	GetApp()->m_Tool.SetTool(Tool::ToolType::None);
}

void MainWindow::OnToolSelect()
{
	GetApp()->m_Tool.SetTool(Tool::ToolType::Select);
}

void MainWindow::OnToolViewHome()
{
	// Reset view to included entire scene, centered.
	//

	if (mp_DroneWindow)
		mp_DroneWindow->ResetView();
	else if (mp_LidarWindow)
		mp_LidarWindow->ResetView();
	else if (mp_OrthoWindow)
		mp_OrthoWindow->ResetView();
	else if (mp_TerrainWindow)
		mp_TerrainWindow->ResetView();
}

void MainWindow::OnToolViewZoom()
{
	GetApp()->m_Tool.SetTool(Tool::ToolType::ViewZoom);
}

void MainWindow::OnToolViewRotate()
{
	GetApp()->m_Tool.SetTool(Tool::ToolType::ViewRotate);
}

void MainWindow::OnToolViewPan()
{
	GetApp()->m_Tool.SetTool(Tool::ToolType::ViewPan);
}

void MainWindow::OnToolViewZoomIn()
{
}

void MainWindow::OnToolViewZoomOut()
{
}

void MainWindow::OnToolDistance()
{
	GetApp()->m_Tool.SetTool(Tool::ToolType::Distance);
}

void MainWindow::OnToolArea()
{
	GetApp()->m_Tool.SetTool(Tool::ToolType::Area);
}

void MainWindow::OnToolLight()
{
	GetApp()->m_Tool.SetTool(Tool::ToolType::Light);
}

void MainWindow::OnToolContour()
{
	if (mp_TerrainWindow)
	{
		mp_TerrainWindow->Contour();
	}
}

void MainWindow::OnToolElev()
{
	if (mp_TerrainWindow)
	{
		mp_TerrainWindow->OnElevation();
	}
}

void MainWindow::OnToolProfile()
{
	GetApp()->m_Tool.SetTool(Tool::ToolType::Profile);
}

void MainWindow::OnDroneProc()
{
	// Process drone imagery.
	//

	DroneProcDlg dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		DroneProc* pProc = new DroneProc();

//TODO:
//works, but intent is to run async using signals & slots like geoindex
		pProc->Run(dlg.GetInitStage());

		delete pProc;
	}
}

void MainWindow::OnLidarDxm()
{
	// Create DTM/DSM from lidar point cloud.
	//

	LidarModelDlg dlg(this);
	if (mp_LidarWindow)
	{
		XString str = mp_LidarWindow->GetLasFile()->GetFileName();
		dlg.SetSourceFile(str.c_str());
	}
	if (dlg.exec() == QDialog::Accepted)
	{
		LidarModel* pProc = new LidarModel();

		pProc->SetSourceFile(dlg.GetSourceFile().c_str());
		pProc->SetOutputFolder(dlg.GetOutputFolder().c_str());
		pProc->SetResolution(dlg.GetResolution());

		if (dlg.IsDsmSelected())
			pProc->CreateDSM();
		if (dlg.IsDtmSelected())
			pProc->CreateDTM();

		delete pProc;
	}
}

void MainWindow::OnColorIndexChanged(int /* index */)
{
	// User changed color scale selection.
	//

	if (mp_LidarWindow)
	{
		LidarWindow::LidarAttr attr = (LidarWindow::LidarAttr)mp_cboAttrLidar->currentData().toInt();
		XString scale = XString::CombinePath(GetApp()->GetAppDataPath(), XString(mp_cboColorLidar->currentText()));
		mp_LidarWindow->SetColorScale(attr, scale.c_str());
	}
	else if (mp_TerrainWindow)
	{
		XString scale = mp_cboColorTerrain->currentText();
		if (scale != ORTHO_KEY)
			scale = XString::CombinePath(GetApp()->GetAppDataPath(), scale.c_str());
		mp_TerrainWindow->SetColorScale(scale.c_str());
	}

	// remember most recent selection
	QSettings settings(ORG_KEY, APP_KEY);
	settings.setValue("", QStringList());
}

void MainWindow::OnToolExport()
{
}

void MainWindow::OnFileClear()
{
	// Clear MRU list.
	//

	QSettings settings(ORG_KEY, APP_KEY);
	settings.setValue("recentFileList", QStringList());

	UpdateRecentFileActions();
}

void MainWindow::OnProjectChanged()
{
	// Signals any change in project.
	//

	UpdateChildWindows();
	UpdateWindowTitle();
}

void MainWindow::OnViewChanged(AeroMap::ViewType view, int subItem)
{
	// Signals change in active view.
	//

	// close existing windows
	CloseChildWindows();

	mp_toolBarDrone->hide();
	mp_toolBarLidar->hide();
	mp_toolBarOrtho->hide();
	mp_toolBarTerrain->hide();

	switch (view)
	{
	case AeroMap::ViewType::Drone:
		{
			assert(mp_DroneWindow == nullptr);

			mp_DroneWindow = new DroneWindow(this);
			mp_mdiArea->addSubWindow(mp_DroneWindow);

			mp_DroneWindow->showMaximized();
			mp_toolBarDrone->show();
		}
		break;
	case AeroMap::ViewType::Lidar:
		{
			assert(mp_LidarWindow == nullptr);

			if ((subItem > -1) && (subItem < GetProject().GetLidarFileCount()))
			{
				// always create/show the window
				mp_LidarWindow = new LidarWindow(this, subItem);
				mp_mdiArea->addSubWindow(mp_LidarWindow);

				mp_LidarWindow->showMaximized();
				mp_toolBarLidar->show();
			}
		}
		break;
	case AeroMap::ViewType::Orthophoto:
		{
			assert(mp_OrthoWindow == nullptr);

			mp_OrthoWindow = new OrthoWindow(this);
			mp_mdiArea->addSubWindow(mp_OrthoWindow);

			mp_OrthoWindow->showMaximized();
			mp_toolBarOrtho->show();
		}
		break;
	case AeroMap::ViewType::Terrain:
		{
			assert(mp_TerrainWindow == nullptr);

			//TODO:
			mp_TerrainWindow = new TerrainWindow(this, 1);
			mp_mdiArea->addSubWindow(mp_TerrainWindow);

			mp_TerrainWindow->showMaximized();
			mp_toolBarTerrain->show();
		}
		break;
	case AeroMap::ViewType::None:
		break;
	}

	UpdateMenus();
	//SetCurrentFile(pathName);
	statusBar()->showMessage(tr("View activated"), 2000);
}
