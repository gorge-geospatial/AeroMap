#ifndef LIGHTWINDOW_H
#define LIGHTWINDOW_H

#include "Light.h"
#include "Material.h"

#include <QtGui>
#include <QGridLayout>

#include "ui_light.h"

class LightWindow : public QWidget,
			        private Ui::LightForm
{
	Q_OBJECT

public:

	enum class Tab
	{
		None,
		Light,
		Material
	};

public:

	explicit LightWindow(QWidget* parent = nullptr);
	~LightWindow();

	void SetDefaultTab(Tab tab);

private slots:

	void OnCboLightIndexChanged(int index); 
	void OnLightChanged();
	void OnLightSliderChanged();
	void OnLightSync();
	
	void OnCboMaterialIndexChanged(int index);
	void OnMatChanged();
	void OnMatSliderChanged();
	void OnMatAmbientSync();
	void OnMatDiffuseSync();
	void OnMatSpecularSync();
	
	void OnClose();

private:

	// QWidget
	virtual void closeEvent(QCloseEvent* event) override;

	void LoadSelectedLight();
	void LoadSelectedMaterial();
	Light* GetLight();
	Material* GetMaterial();

	bool mb_ChangeMask;
};

#endif // #ifndef LIGHTWINDOW_H
