#ifndef METADATADLG_H
#define METADATADLG_H

#include "XString.h"

#include <QDialog>
#include "ui_metadata.h"

class MetaDataDlg : public QDialog,
					private Ui::MetaDataDlg
{
	Q_OBJECT

public:

	explicit MetaDataDlg(QWidget* parent, const char* title);
	~MetaDataDlg();

	void SetMetaData(XString& strMeta);
	XString GetMetaData() const;

private slots:

	void OnClose();
	void OnClipBoard();

};

#endif // METADATADLG_H
