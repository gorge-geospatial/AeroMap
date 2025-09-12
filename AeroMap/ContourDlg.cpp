// ContourDlg.cpp
// Parameter dialog for contouring algorithm.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "ScaleColor.h"
#include "ContourDlg.h"

#include <QtGui>
#include <QMessageBox>
#include <QColorDialog>

ContourDlg::ContourDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	// make dialog fixed size
	setFixedSize(size());
	// remove question mark from title bar
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	LoadScaleColorList();

	// make scene exactly same size as view
	m_sceneColor.setSceneRect(0.0, 0.0, graphicsViewColor->width(), graphicsViewColor->height());

	// load data
	QSettings settings(ORG_KEY, APP_KEY);
	spinInterval->setValue(settings.value(CONTOUR_INTERVAL_KEY, 100.0).toDouble());
	spinStart->setValue(settings.value(CONTOUR_START_KEY, 100.0).toDouble());

	QColor color = settings.value(CONTOUR_COLOR_KEY, 0).value<QColor>();
	m_sceneColor.setBackgroundBrush(color);
	graphicsViewColor->setScene(&m_sceneColor);

	OnColorSource();

	// signals and slots
	verify_connect(optColorSolid, SIGNAL(clicked()), this, SLOT(OnColorSource()));
	verify_connect(optColorScale, SIGNAL(clicked()), this, SLOT(OnColorSource()));
	verify_connect(cmdClose, SIGNAL(clicked()), this, SLOT(OnClose()));
	verify_connect(cmdCreate, SIGNAL(clicked()), this, SLOT(OnCreate()));
	verify_connect(cmdColor, SIGNAL(clicked()), this, SLOT(OnColor()));
}

ContourDlg::~ContourDlg()
{
}

void ContourDlg::LoadScaleColorList()
{
	// Load all defined color scales.
	//

	cboColorScale->clear();
	cboColorScale->addItem("");

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
				cboColorScale->addItem(fileName.c_str());
		}
	}
}

void ContourDlg::SetElevationRange(double minElev, double maxElev)
{
	char buf[40];
	sprintf(buf, "%0.1f", minElev);
	labMinElevation->setText(buf);
	sprintf(buf, "%0.1f", maxElev);
	labMaxElevation->setText(buf);
}

double ContourDlg::GetInterval()
{ 
	return spinInterval->value();
}

double ContourDlg::GetStart()
{ 
	return spinStart->value();
}

PixelType ContourDlg::GetContourColor()
{
	PixelType pix;
	pix.R = m_sceneColor.backgroundBrush().color().red();
	pix.G = m_sceneColor.backgroundBrush().color().green();
	pix.B = m_sceneColor.backgroundBrush().color().blue();
	return pix;
}

XString ContourDlg::GetContourScale()
{
	XString strScaleColor;

	if (cboColorScale->currentIndex() > 0)
	{
		strScaleColor = XString::CombinePath(GetApp()->GetAppDataPath().c_str(), XString(cboColorScale->currentText()).c_str());
	}
	
	return strScaleColor;
}

void ContourDlg::OnCreate()
{
    // save data
	QSettings settings(ORG_KEY, APP_KEY);
	settings.setValue(CONTOUR_INTERVAL_KEY, spinInterval->value());
	settings.setValue(CONTOUR_START_KEY, spinStart->value());
	settings.setValue(CONTOUR_COLOR_KEY, m_sceneColor.backgroundBrush().color());

    accept();
}

void ContourDlg::OnColorSource()
{
	if (optColorSolid->isChecked())
	{
		cmdColor->setEnabled(true);
		cboColorScale->setEnabled(false);
	}
	else
	{
		cboColorScale->setEnabled(true);
		cmdColor->setEnabled(false);
	}
}

void ContourDlg::OnColor()
{
	QColor colorInit = m_sceneColor.backgroundBrush().color();
	QColor color = QColorDialog::getColor(colorInit, this, "Select Contour Color");
	if (color.isValid())
	{
		QBrush brushBack(color);
		m_sceneColor.setBackgroundBrush(brushBack);
	}
}

void ContourDlg::OnClose()
{
	reject();
}
