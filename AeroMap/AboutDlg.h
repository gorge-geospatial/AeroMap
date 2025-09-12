#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#include <QDialog>
#include "ui_about.h"

class AboutDlg : public QDialog,
				 private Ui::AboutDlg
{
	Q_OBJECT

public:

	explicit AboutDlg(QWidget* parent = nullptr);
	~AboutDlg();

private slots:

	void OnClose();
};

#endif // #ifndef ABOUTDLG_H
