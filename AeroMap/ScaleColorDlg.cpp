#include "AeroMap.h"			// application header
#include "ScaleColorDlg.h"

#include <QtGui>
#include <QFileDialog>

ScaleColorDlg::ScaleColorDlg(QWidget* parent)
	: QDialog(parent)
{
    setupUi(this);
	// make dialog fixed size
	setFixedSize(size());
	// remove question mark from title bar
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	spinBandCount->setRange(ScaleColor::GetMinBandCount(), ScaleColor::GetMaxBandCount());

	// signals and slots
	verify_connect(cmdClose, SIGNAL(clicked()), this, SLOT(OnClose()));
	verify_connect(cmdSave, SIGNAL(clicked()), this, SLOT(OnSave()));
	verify_connect(cmdSaveAs, SIGNAL(clicked()), this, SLOT(OnSaveAs()));
	verify_connect(cmdClear, SIGNAL(clicked()), this, SLOT(OnClear()));
	verify_connect(cmdDigitize, SIGNAL(clicked()), this, SLOT(OnDigitize()));
	verify_connect(cmdScaleFile, SIGNAL(clicked()), this, SLOT(OnSelectScaleFile()));
	verify_connect(spinBandCount, SIGNAL(valueChanged(int)), this, SLOT(OnBandCountChanged(int)));
}

ScaleColorDlg::~ScaleColorDlg()
{
}

void ScaleColorDlg::LoadScale(const char* fileName)
{
	if (fileName == nullptr)
		return;
	if (QFile::exists(fileName) == false)
		return;

	graphicsViewScale->Load(fileName);
	txtScaleDesc->setText(graphicsViewScale->GetDescription());
	spinBandCount->setValue(graphicsViewScale->GetBandCount());

	XString rootName = XString(fileName).GetFileName();
	SetCaption(rootName);
}

void ScaleColorDlg::ClearForm()
{
	txtScaleFile->clear();
	txtScaleDesc->clear();
	spinBandCount->setValue(ScaleColor::GetDefaultBandCount());

	graphicsViewScale->Clear();
}

void ScaleColorDlg::OnBandCountChanged(int bandCount)
{
	graphicsViewScale->SetBandCount(bandCount);
}

void ScaleColorDlg::OnSave()
{
	if (graphicsViewScale->GetFileName().GetLength() == 0)
	{
		OnSaveAs();
	}
	else
	{
		graphicsViewScale->SetDescription(XString(txtScaleDesc->text()));
		graphicsViewScale->Save();
	}
}

void ScaleColorDlg::OnSaveAs()
{
	QString fileName = QFileDialog::getSaveFileName(
		this, 
		tr("Save Color Scale"), 
		GetApp()->GetAppDataPath().c_str(),
		GetFileFilter());

	if (fileName.length() > 0)
	{
		graphicsViewScale->SetDescription(XString(txtScaleDesc->text()));
		graphicsViewScale->Save(XString(fileName).c_str());
	}
}

void ScaleColorDlg::OnClear()
{
	ClearForm();
}

void ScaleColorDlg::OnClose()
{
    reject();
}

void ScaleColorDlg::OnDigitize()
{
	// Digitize a graphic color scale.

	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr("Select Image File"),
		GetApp()->GetAppDataPath().c_str(),
		tr("Color Scales (*.png *.jpg *.bmp)"));

	if (!fileName.isEmpty())
	{
		graphicsViewScale->Digitize(fileName.toLatin1().data());
	}
}

void ScaleColorDlg::OnSelectScaleFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this, 
		tr("Select Color Scale"), 
		GetApp()->GetAppDataPath().c_str(),
		GetFileFilter());

	if (!fileName.isEmpty())
	{
		txtScaleFile->setText(fileName);
		LoadScale(XString(fileName).c_str());
	}
}

QString ScaleColorDlg::GetFileFilter() const
{
	QString filter = tr("Color Scales (*.");
	filter += ScaleColor::GetDefaultExt();
	filter += tr(")");
	return filter;
}

void ScaleColorDlg::SetCaption(const char* fileName)
{
	XString str = XString::Format("Edit Color Scale [%s]", fileName);
	setWindowTitle(str.c_str());
}