#ifndef SCALECOLORDLG_H
#define SCALECOLORDLG_H

#include "ScaleColor.h"

#include <QDialog>
#include "ui_scalecolor.h"

class ScaleColorDlg : public QDialog,
					  private Ui::ScaleColorDlg
{
	Q_OBJECT

public:

	explicit ScaleColorDlg(QWidget* parent = nullptr);
	~ScaleColorDlg();

private:

	QGraphicsScene m_Scene;		// scene backing graphicsView

private slots:

	void OnClose();
	void OnSave();
	void OnSaveAs();
	void OnClear();
	void OnDigitize();
	void OnSelectScaleFile();
	void OnBandCountChanged(int);

private:
	
	void ClearForm();
	void LoadScale(const char* fileName);
	QString GetFileFilter() const;
	void SetCaption(const char* fileName);
};

#endif // SCALECOLORDLG_H
