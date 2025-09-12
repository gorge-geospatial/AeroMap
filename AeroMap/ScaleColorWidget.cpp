// ScaleColorWidget.cpp
// Custom widget for editing color scales.
//

#include <QMessageBox>

#include <assert.h>

#include "ImageFile.h"
#include "ScaleColorWidget.h"

#define SCALE_HEIGHT	300.0			// fixed visual scale bar height
#define SCALE_WIDTH		140.0			// fixed visual scale bar width
#define SCALE_LEFT		120.0
#define SCALE_RIGHT		(SCALE_LEFT + SCALE_WIDTH)
#define SCALE_TOP		45.0
#define SCALE_BOTTOM	SCALE_TOP + SCALE_HEIGHT
#define HANDLE_LEN		50.0
#define BAND_BUF		(UInt8(8))		// how close one band can get to it's neighbors, whole percent

#define PEN_COLOR  QColor(0,0,0)
#define BACK_COLOR QColor(228,228,255)

ScaleColorWidget::ScaleColorWidget(QWidget* pParent)
	: QGraphicsView(pParent)
	, mp_Scale(nullptr)
	, mp_pixmapScale(nullptr)
	, mp_pixmapWater(nullptr)
	, mb_ChangeMask(false)
	, mi_BandHandle(-1)
	, mp_chkWaterColor(nullptr)
	, mp_spinWaterR(nullptr)
	, mp_spinWaterG(nullptr)
	, mp_spinWaterB(nullptr)
{
	CreateControls();
}

ScaleColorWidget::~ScaleColorWidget()
{
	// before clearing scene, delete and clear the scale image ptr, unlike
	// other graphicsview items, this is recreated inside UpdateGraphics()
	delete mp_pixmapScale;
	mp_pixmapScale = nullptr;

	delete mp_pixmapWater;
	mp_pixmapWater = nullptr;

	m_scene.clear();

	delete mp_Scale;
	mp_Scale = nullptr;

	delete mp_chkWaterColor;
	mp_chkWaterColor = nullptr;
	
	delete mp_spinWaterR;
	delete mp_spinWaterG;
	delete mp_spinWaterB;
}

void ScaleColorWidget::CreateScene()
{
	// Recreate entire scene.
	//

	delete mp_pixmapScale;
	mp_pixmapScale = nullptr;

	delete mp_pixmapWater;
	mp_pixmapWater = nullptr;

	m_scene.clear();

	QBrush brushBack(BACK_COLOR, Qt::SolidPattern);
	QPen penQuad(PEN_COLOR, Qt::SolidLine);

	// make scene exactly same size as view
	m_scene.setSceneRect(0.0, 0.0, width(), height());
	m_scene.setBackgroundBrush(brushBack);

	if (mp_Scale)
	{
		m_scene.addRect(SCALE_LEFT - 1, SCALE_TOP - 1, SCALE_WIDTH + 1, SCALE_HEIGHT + 1, penQuad, brushBack);

		mv_txtBand.clear();
		mv_line.clear();

		for (int i = 0; i < mp_Scale->GetBandCount(); ++i)
		{
			// label the bands
			char buf[32];
			sprintf(buf, "Band %d", i);
			mv_txtBand.push_back(m_scene.addSimpleText(buf));

			// add ticks/drag handle lines
			mv_line.push_back(m_scene.addLine(SCALE_RIGHT + 4.0, GetBandY(i), SCALE_RIGHT + HANDLE_LEN, GetBandY(i)));
			if (i > 0 && i < mp_Scale->GetBandCount() - 1)
				mv_line[i]->setCursor(Qt::SizeVerCursor);
			else
				mv_line[i]->setCursor(Qt::ArrowCursor);
		}

		UpdateGraphics();
	}
	setScene(&m_scene);
	setMouseTracking(true);
}

void ScaleColorWidget::UpdateGraphics()
{
	// Update graphics that don't require
	// a recreation of entire scene.
	//

	const double COL1 = 60.0;
	const double COL2 = 105.0;
	const double COL3 = 150.0;

	for (int i = 0; i < mp_Scale->GetBandCount(); ++i)
	{
		mv_txtBand[i]->setPos(SCALE_LEFT - 90.0, GetBandY(i) - mv_txtBand[i]->boundingRect().height() / 2 - 1.0);
		if (i > 0 && i < mp_Scale->GetBandCount() - 1)
			mv_line[i]->setLine(SCALE_RIGHT + 4.0, GetBandY(i), SCALE_RIGHT + HANDLE_LEN, GetBandY(i));
	}

	QImage image(SCALE_WIDTH, SCALE_HEIGHT, QImage::Format_RGB32);
	for (int y = 0; y < image.height(); ++y)
	{
		double sf = (double)(image.height() - y - 1) / (double)(image.height() - 1);
		PixelType pix = mp_Scale->GetColorByHeight(sf);
		QPen pen;
		pen.setColor(QColor(pix.R, pix.G, pix.B));
		for (int x = 0; x < image.width(); ++x)
		{
			PixelType pix = mp_Scale->GetColorByHeight(sf);
			image.setPixel(x, y, qRgb(pix.R, pix.G, pix.B));
		}
	}

	if (mp_pixmapScale)
		m_scene.removeItem(mp_pixmapScale);
	QPixmap pixmap = QPixmap::fromImage(image);
	mp_pixmapScale = m_scene.addPixmap(pixmap);
	mp_pixmapScale->setPos(SCALE_LEFT, SCALE_TOP);

	// update band labels
	char buf[32];
	for (int i = 0; i < mp_Scale->GetBandCount(); ++i)
	{
		sprintf(buf, "Band %d  (%3u%%)", i, mp_Scale->GetBandHeight(i));
		mv_txtBand[i]->setText(buf);
	}

	mb_ChangeMask = true;

	// position color input controls
	ShowBandControls();
	int dy = mv_spinR[0]->height() / 2;
	for (int i = 0; i < mp_Scale->GetBandCount(); ++i)
	{
		mv_spinR[i]->move(SCALE_RIGHT + COL1, GetBandY(i) - dy); mv_spinR[i]->show();
		mv_spinG[i]->move(SCALE_RIGHT + COL2, GetBandY(i) - dy); mv_spinG[i]->show();
		mv_spinB[i]->move(SCALE_RIGHT + COL3, GetBandY(i) - dy); mv_spinB[i]->show();
		
		mv_spinR[i]->setValue(mp_Scale->GetBandR(i));
		mv_spinG[i]->setValue(mp_Scale->GetBandG(i));
		mv_spinB[i]->setValue(mp_Scale->GetBandB(i));
	}

	// render water color
	mp_chkWaterColor->setChecked(mp_Scale->HasWaterColor());
	mp_chkWaterColor->move(SCALE_LEFT, SCALE_BOTTOM + mp_chkWaterColor->height());
	mp_chkWaterColor->show();

	QImage imageWater(SCALE_WIDTH, 50.0, QImage::Format_RGB32);
	imageWater.fill(QColor(mp_Scale->GetWaterColor().R, mp_Scale->GetWaterColor().G, mp_Scale->GetWaterColor().B));
	if (mp_pixmapWater)
		m_scene.removeItem(mp_pixmapWater);
	mp_pixmapWater = m_scene.addPixmap(QPixmap::fromImage(imageWater));
	mp_pixmapWater->setPos(SCALE_LEFT, mp_chkWaterColor->pos().y() + mp_chkWaterColor->height());

	int spinTop = mp_pixmapWater->pos().y() + (mp_pixmapWater->boundingRect().height() - mp_spinWaterR->height()) / 2;
	mp_spinWaterR->move(SCALE_RIGHT + COL1, spinTop);
	mp_spinWaterG->move(SCALE_RIGHT + COL2, spinTop);
	mp_spinWaterB->move(SCALE_RIGHT + COL3, spinTop);

	mp_pixmapWater->setVisible(mp_chkWaterColor->isChecked());

	mp_spinWaterR->setValue(mp_Scale->GetWaterColor().R);
	mp_spinWaterG->setValue(mp_Scale->GetWaterColor().G);
	mp_spinWaterB->setValue(mp_Scale->GetWaterColor().B);

	mp_spinWaterR->setVisible(mp_chkWaterColor->isChecked());
	mp_spinWaterG->setVisible(mp_chkWaterColor->isChecked());
	mp_spinWaterB->setVisible(mp_chkWaterColor->isChecked());
	
	mb_ChangeMask = false;
}

void ScaleColorWidget::SetBandCount(int bandCount)
{
	if (mp_Scale == nullptr)
		return;

	mp_Scale->SetBandCount(bandCount);
	CreateScene();
}

int ScaleColorWidget::GetBandCount()
{
	if (mp_Scale == nullptr)
		return 0;

	return mp_Scale->GetBandCount();
}

double ScaleColorWidget::GetBandY(int band)
{
	// Get y coordinate of given band.
	//

	double bandY = 0.0;

	if (band == 0)
		bandY = SCALE_BOTTOM;
	else if (band == mp_Scale->GetBandCount() - 1)
		bandY = SCALE_TOP;
	else
	{
		double sf = 1.0 - static_cast<double>(mp_Scale->GetBandHeight(band)) * 0.01;
		bandY = SCALE_TOP + SCALE_HEIGHT * sf;
	}

	return bandY;
}

void ScaleColorWidget::Load(const char* fileName)
{
	// Load color scale.
	//

	if (QFile::exists(fileName) == false)
		return;

	delete mp_Scale;
	mp_Scale = new ScaleColor(fileName);

	CreateScene();
}

void ScaleColorWidget::Save(const char* fileName)
{
	if (mp_Scale == nullptr)
		return;

	for (int i = 0; i < mp_Scale->GetBandCount(); ++i)
	{
		mp_Scale->SetBandColor(i, mv_spinR[i]->value(), mv_spinG[i]->value(), mv_spinB[i]->value());
	}

	if (mp_chkWaterColor->isChecked() == true)
	{
		mp_Scale->SetWaterColor(PixelType(mp_spinWaterR->value(), mp_spinWaterG->value(), mp_spinWaterB->value()));
	}
	else
	{
		mp_Scale->ClearWaterColor();
	}

	mp_Scale->Save(fileName);
}

void ScaleColorWidget::Clear()
{
	delete mp_Scale;
	mp_Scale = new ScaleColor();
	UpdateGraphics();
}

void ScaleColorWidget::Digitize(const char* fileName)
{
	// Create new scale by loading colors from image
	// file. It's assumed that image orientation is 
	// vertical, where bottom to top corresponds to 
	// low to high elevations.

	assert(fileName != nullptr);
	if (QFile::exists(fileName) == false)
		return;

	delete mp_Scale;
	mp_Scale = new ScaleColor();
	mp_Scale->RemoveAllBands();

	ImageFile imageFile(fileName);
	int w = imageFile.GetWidth();
	int h = imageFile.GetHeight();

	int bandCount = 5;
	int dy = h / (bandCount - 1);
	int y = h - 1;
	for (int i = 0; i < bandCount; ++i)
	{
		// make sure last entry is exactly
		// on image row 0
		if (i == bandCount - 1)
			y = 0;

		PixelType pix = imageFile.GetPixel(w / 2, y);
		mp_Scale->AddBand(i*25, pix.R, pix.G, pix.B);
		y -= dy;
	}
	
	UpdateGraphics();
}

const char* ScaleColorWidget::GetDescription() const
{
	if (mp_Scale)
		return mp_Scale->GetDescription();

	return "";
}

void ScaleColorWidget::SetDescription(const char* desc)
{
	if (mp_Scale)
		mp_Scale->SetDescription(desc);
}

const XString ScaleColorWidget::GetFileName() const
{
	XString fileName;

	if (mp_Scale)
		fileName = mp_Scale->GetFileName();

	return fileName;
}

void ScaleColorWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::MouseButton::LeftButton)
		mi_BandHandle = GetBandHandle(event);
}

void ScaleColorWidget::mouseReleaseEvent(QMouseEvent* event)
{
	Q_UNUSED(event);

	mi_BandHandle = -1;
}

void ScaleColorWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
}

void ScaleColorWidget::mouseMoveEvent(QMouseEvent* event)
{
	// this event only happens if mouse button down or
	// setMouseTracking(true) has been called
	//

	// this band handle is just for selecting
	// the mouse cursor
	if (GetBandHandle(event) > -1)
		setCursor(Qt::SizeVerCursor);
	else
		unsetCursor();

	// this band handle is for actually moving bands
	if (mi_BandHandle > -1)
	{
		// if mouse down
		if (event->buttons() & Qt::MouseButton::LeftButton)
		{
			//	move it

			double sf = 1.0 - ((double)event->y() - SCALE_TOP) / (SCALE_BOTTOM - SCALE_TOP);
			UInt8 bandHeight = (UInt8)(sf * 100.0);

			if (mi_BandHandle > 0 && mi_BandHandle < mp_Scale->GetBandCount() - 1)
			{
				// don't let it too close to (or overrun) bounding bands
				if ((bandHeight > mp_Scale->GetBandHeight(mi_BandHandle - 1) + BAND_BUF)
				&&  (bandHeight < mp_Scale->GetBandHeight(mi_BandHandle + 1) - BAND_BUF))
					mp_Scale->SetBandHeight(mi_BandHandle, bandHeight);
			}

			UpdateGraphics();
		}
	}
}

void ScaleColorWidget::showEvent(QShowEvent* event)
{
	// Create a default scale & scene when we first show 
	// the widget.
	//

	Q_UNUSED(event);

	delete mp_Scale;
	mp_Scale = new ScaleColor();
	mp_Scale->Create();
	CreateScene();
}

int ScaleColorWidget::GetBandHandle(QMouseEvent* event)
{
	// Return # of band handle at mouse location,
	// or -1 if none.
	//

	for (int i = 1; i < mv_line.size() - 1; ++i)
	{
		const double dv = 4.0;

		QRectF rect = mv_line[i]->boundingRect();
		rect.adjust(0.0, -dv, 0.0, dv);
		if (rect.contains(event->x(), event->y()))
			return i;

	}

	return -1;
}

void ScaleColorWidget::ShowBandControls()
{
	// Show number of controls that matches
	// backing scale.

	for (int i = 0; i < mp_Scale->GetBandCount(); ++i)
	{
		mv_txtBand[i]->show();

		mv_line[i]->show();

		mv_spinR[i]->show();
		mv_spinG[i]->show();
		mv_spinB[i]->show();
	}

	for (int i = mp_Scale->GetBandCount(); i < ScaleColor::GetMaxBandCount(); ++i)
	{
		// scene-dependent controls don't need to be hidden,
		// only the # that matches backing scale exist

		mv_spinR[i]->hide();
		mv_spinG[i]->hide();
		mv_spinB[i]->hide();
	}
}

void ScaleColorWidget::OnValueChanged(int value)
{
	// Occurs when any spin box changes.
	//

	Q_UNUSED(value);

	if (mb_ChangeMask)
		return;
	if (mp_Scale == nullptr)
		return;

	for (int i = 0; i < mp_Scale->GetBandCount(); ++i)
	{
		mp_Scale->SetBandColor(i, mv_spinR[i]->value(), mv_spinG[i]->value(), mv_spinB[i]->value());
	}
	
	mp_Scale->SetWaterColor(PixelType(mp_spinWaterR->value(), mp_spinWaterG->value(), mp_spinWaterB->value()));

	UpdateGraphics();
}

void ScaleColorWidget::OnChkWaterColorChanged(int state)
{
	if (mb_ChangeMask)
		return;

	if (state == Qt::Checked)
	{
		mp_Scale->SetWaterColor(PixelType(mp_spinWaterR->value(), mp_spinWaterG->value(), mp_spinWaterB->value()));
	}
	else
	{
		mp_Scale->ClearWaterColor();
	}

	UpdateGraphics();
}

void ScaleColorWidget::CreateControls()
{
	// Create max # of controls. Only show
	// active subset.

	// verify # of controls created matches max # of bands
	assert(ScaleColor::GetMaxBandCount() == 9);

	mv_spinR.clear();
	mv_spinG.clear();
	mv_spinB.clear();

	for (int i = 0; i < ScaleColor::GetMaxBandCount(); ++i)
	{
		mv_spinR.push_back(new QSpinBox(this));
		mv_spinG.push_back(new QSpinBox(this));
		mv_spinB.push_back(new QSpinBox(this));

		mv_spinR[i]->hide();
		mv_spinG[i]->hide();
		mv_spinB[i]->hide();

		mv_spinR[i]->setFixedSize(42, 22);
		mv_spinG[i]->setFixedSize(42, 22);
		mv_spinB[i]->setFixedSize(42, 22);

		mv_spinR[i]->setRange(0, 255);
		mv_spinG[i]->setRange(0, 255);
		mv_spinB[i]->setRange(0, 255);

		mv_spinR[i]->setAlignment(Qt::AlignRight);
		mv_spinG[i]->setAlignment(Qt::AlignRight);
		mv_spinB[i]->setAlignment(Qt::AlignRight);

		verify_connect(mv_spinR[i], SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged(int)));
		verify_connect(mv_spinG[i], SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged(int)));
		verify_connect(mv_spinB[i], SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged(int)));
	}

	mp_chkWaterColor = new QCheckBox(this);
	mp_chkWaterColor->setText(tr("Water Color"));
	mp_chkWaterColor->setChecked(false);
	mp_chkWaterColor->setToolTip(tr("Interpret areas of zero elevation as water & use this color."));

	mp_spinWaterR = new QSpinBox(this);
	mp_spinWaterG = new QSpinBox(this);
	mp_spinWaterB = new QSpinBox(this);

	mp_spinWaterR->setFixedSize(42, 22);
	mp_spinWaterG->setFixedSize(42, 22);
	mp_spinWaterB->setFixedSize(42, 22);

	mp_spinWaterR->setRange(0, 255);
	mp_spinWaterG->setRange(0, 255);
	mp_spinWaterB->setRange(0, 255);

	mp_spinWaterR->setAlignment(Qt::AlignRight);
	mp_spinWaterG->setAlignment(Qt::AlignRight);
	mp_spinWaterB->setAlignment(Qt::AlignRight);

	verify_connect(mp_chkWaterColor, SIGNAL(stateChanged(int)), this, SLOT(OnChkWaterColorChanged(int)));
	verify_connect(mp_spinWaterR, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged(int)));
	verify_connect(mp_spinWaterG, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged(int)));
	verify_connect(mp_spinWaterB, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged(int)));
}
