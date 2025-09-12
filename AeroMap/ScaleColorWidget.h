#ifndef SCALECOLORWIDGET_H
#define SCALECOLORWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>
#include <QPixmap>
#include <QSpinBox>
#include <QCheckBox>

#include "AeroMap.h"		// application header
#include "ScaleColor.h"		// color scale

class ScaleColorWidget : public QGraphicsView
{
	Q_OBJECT

public:

	ScaleColorWidget(QWidget* pParent = nullptr);
	virtual ~ScaleColorWidget();

	void Load(const char* fileName);
	void Save(const char* fileName = nullptr);
	void Clear();
	void SetBandCount(int bandCount);
	int  GetBandCount();

	const XString GetFileName() const;
	const char* GetDescription() const;
	void SetDescription(const char* desc);

	void Digitize(const char* fileName);

protected:

	// QWidget overrides
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void showEvent(QShowEvent *event) override;

private slots:

	void OnValueChanged(int value);
	void OnChkWaterColorChanged(int);

private:
	
	ScaleColor* mp_Scale;		// backing color scale
	QGraphicsScene m_scene;		// backing scene
	bool mb_ChangeMask;			// true => don't process change events
	int  mi_BandHandle;			// selected band handle

	std::vector<QGraphicsSimpleTextItem*> mv_txtBand;		// band labels
	std::vector<QSpinBox*> mv_spinR;						// band colors
	std::vector<QSpinBox*> mv_spinG;
	std::vector<QSpinBox*> mv_spinB;
	std::vector<QGraphicsLineItem*> mv_line;				// band handle lines
	QCheckBox* mp_chkWaterColor;							// option to use separate color for water
	QSpinBox* mp_spinWaterR;								// water color components
	QSpinBox* mp_spinWaterG;
	QSpinBox* mp_spinWaterB;

	QGraphicsPixmapItem* mp_pixmapScale;		// bitmap used for rendering scale image
	QGraphicsPixmapItem* mp_pixmapWater;		// bitmap used for rendering water color

private:

	void   CreateScene();
	void   CreateControls();
	double GetBandY(int band);
	int    GetBandHandle(QMouseEvent* event);
	void   UpdateGraphics();
	void   ShowBandControls();
};

#endif // #ifndef SCALECOLORWIDGET_H
