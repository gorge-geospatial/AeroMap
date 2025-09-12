#ifndef LIDARMODELDLG_H	
#define LIDARMODELDLG_H	

#include "XString.h"

#include <QDialog>
#include "ui_lidar_model.h"

class LidarModelDlg : public QDialog,
					  private Ui::LidarModelDlg
{
	Q_OBJECT

public:

	explicit LidarModelDlg(QWidget* parent = nullptr);
	~LidarModelDlg();

	void    SetSourceFile(const char* fileName);
	XString GetSourceFile();
	XString GetOutputFolder();
	double  GetResolution();

	bool IsDsmSelected();
	bool IsDtmSelected();

private slots:

	void OnCreate();
	void OnClose();
	void OnSelectSourceFile();
	void OnSelectOutputPath();
	void OnDtmClicked();
	void OnDsmClicked();

private:

	void EnableCreate();
};

#endif // #ifndef LIDARMODELDLG_H	
