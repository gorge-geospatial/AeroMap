#ifndef CONFIGDLG_H
#define CONFIGDLG_H

#include <QDialog>
#include <QGraphicsScene>
#include "ui_config.h"

class ConfigDlg : public QDialog,
				  private Ui::ConfigDlg
{
	Q_OBJECT

public:

	explicit ConfigDlg(QWidget* parent = nullptr);
	~ConfigDlg();

private slots:

	void OnClose();
	void OnSave();
	void OnSelectAppDataPath();
};

#endif // #ifndef CONFIGDLG_H
