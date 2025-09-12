#ifndef DRONEPROCDLG_H
#define DRONEPROCDLG_H

#include "XString.h"
#include "Stage.h"

#include <QDialog>
#include "ui_drone_proc.h"

class DroneProcDlg : public QDialog,
					 private Ui::DroneProcDlg
{
	Q_OBJECT

public:

	explicit DroneProcDlg(QWidget* parent = nullptr);
	~DroneProcDlg();

	Stage::Id GetInitStage();

private slots:

	void OnRun();
	void OnClose();

private:

	void LoadComboBoxes();
};

#endif // #ifndef DRONEPROCDLG_H
