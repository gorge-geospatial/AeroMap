#ifndef ELEVATIONDLG_H
#define ELEVATIONDLG_H

#include "AeroMap.h"			// application header

#include <QDialog>
#include "ui_elevation.h"

class ElevationDlg : public QDialog,
					 private Ui::ElevationDlg
{
	Q_OBJECT

public:

	explicit ElevationDlg(QWidget* parent = nullptr);
	~ElevationDlg();

	bool IsSmooth() { return optSmooth->isChecked(); }
	bool IsDelta() { return optDelta->isChecked(); }
	double GetDelta() { return spinDelta->value(); }
	double GetScaleFactor() { return spinScale->value(); }

private slots:

	void OnApply();
	void OnClose();
	void OnOptDeltaClicked();
	void OnOptScaleClicked();
	void OnOptSmoothClicked();
};

#endif // #ifndef ELEVATIONDLG_H
