#ifndef CONTOURDLG_H
#define CONTOURDLG_H

#include "AeroMap.h"			// application header

#include <QDialog>
#include <QGraphicsScene>
#include "ui_contour.h"

class ContourDlg : public QDialog,
				   private Ui::ContourDlg
{
	Q_OBJECT

public:

	explicit ContourDlg(QWidget* parent = nullptr);
	~ContourDlg();

	void SetElevationRange(double minElev, double maxElev);

	double GetInterval();
	double GetStart();
	PixelType GetContourColor();
	XString GetContourScale();

private:

	QGraphicsScene m_sceneColor;

private slots:

	void OnColorSource();
	void OnCreate();
	void OnColor();
	void OnClose();

private:

	void LoadScaleColorList();
};

#endif // #ifndef CONTOURDLG_H
