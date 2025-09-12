// LightWindow.cpp
// Light/Material property editor
//

#include "AeroMap.h"			// Application header
#include "GLManager.h"
#include "MainWindow.h"
#include "LightWindow.h"

LightWindow::LightWindow(QWidget* parent)
	: QWidget(parent)
	, mb_ChangeMask(false)
{
	setupUi(this);
	setMinimumSize(235, 200);

	tabWidget->setCurrentIndex(0);

	cboLight->clear();
	for (int i = 0; i < GLManager::GetLightCount(); ++i)
	{
		char buf[16] = { 0 };
		sprintf(buf, "%d", i);
		cboLight->addItem(buf);
	}
	LoadSelectedLight();

	cboMaterial->clear();
	for (int i = 0; i < GLManager::GetMaterialCount(); ++i)
	{
		cboMaterial->addItem(GLManager::GetMaterial(i)->GetName().c_str(), i);
	}
	LoadSelectedMaterial();

	verify_connect(cmdClose, SIGNAL(clicked()), this, SLOT(OnClose()));

	verify_connect(cboLight, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCboLightIndexChanged(int)));

	verify_connect(spinLightR, SIGNAL(valueChanged(double)), this, SLOT(OnLightChanged()));
	verify_connect(spinLightG, SIGNAL(valueChanged(double)), this, SLOT(OnLightChanged()));
	verify_connect(spinLightB, SIGNAL(valueChanged(double)), this, SLOT(OnLightChanged()));

	verify_connect(sliderLightR, SIGNAL(sliderMoved(int)), this, SLOT(OnLightSliderChanged()));
	verify_connect(sliderLightG, SIGNAL(sliderMoved(int)), this, SLOT(OnLightSliderChanged()));
	verify_connect(sliderLightB, SIGNAL(sliderMoved(int)), this, SLOT(OnLightSliderChanged()));
	verify_connect(chkLightSync, SIGNAL(clicked()), this, SLOT(OnLightSync()));

	verify_connect(cboMaterial, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCboMaterialIndexChanged(int)));

	verify_connect(spinMatAmbientR, SIGNAL(valueChanged(double)), this, SLOT(OnMatChanged()));
	verify_connect(spinMatAmbientG, SIGNAL(valueChanged(double)), this, SLOT(OnMatChanged()));
	verify_connect(spinMatAmbientB, SIGNAL(valueChanged(double)), this, SLOT(OnMatChanged()));

	verify_connect(spinMatDiffuseR, SIGNAL(valueChanged(double)), this, SLOT(OnMatChanged()));
	verify_connect(spinMatDiffuseG, SIGNAL(valueChanged(double)), this, SLOT(OnMatChanged()));
	verify_connect(spinMatDiffuseB, SIGNAL(valueChanged(double)), this, SLOT(OnMatChanged()));

	verify_connect(spinMatSpecularR, SIGNAL(valueChanged(double)), this, SLOT(OnMatChanged()));
	verify_connect(spinMatSpecularG, SIGNAL(valueChanged(double)), this, SLOT(OnMatChanged()));
	verify_connect(spinMatSpecularB, SIGNAL(valueChanged(double)), this, SLOT(OnMatChanged()));

	verify_connect(spinMatShininess, SIGNAL(valueChanged(double)), this, SLOT(OnMatChanged()));

	verify_connect(sliderMatAmbientR, SIGNAL(sliderMoved(int)), this, SLOT(OnMatSliderChanged()));
	verify_connect(sliderMatAmbientG, SIGNAL(sliderMoved(int)), this, SLOT(OnMatSliderChanged()));
	verify_connect(sliderMatAmbientB, SIGNAL(sliderMoved(int)), this, SLOT(OnMatSliderChanged()));
	verify_connect(chkMatAmbientSync, SIGNAL(clicked()), this, SLOT(OnMatAmbientSync()));

	verify_connect(sliderMatDiffuseR, SIGNAL(sliderMoved(int)), this, SLOT(OnMatSliderChanged()));
	verify_connect(sliderMatDiffuseG, SIGNAL(sliderMoved(int)), this, SLOT(OnMatSliderChanged()));
	verify_connect(sliderMatDiffuseB, SIGNAL(sliderMoved(int)), this, SLOT(OnMatSliderChanged()));
	verify_connect(chkMatDiffuseSync, SIGNAL(clicked()), this, SLOT(OnMatDiffuseSync()));

	verify_connect(sliderMatSpecularR, SIGNAL(sliderMoved(int)), this, SLOT(OnMatSliderChanged()));
	verify_connect(sliderMatSpecularG, SIGNAL(sliderMoved(int)), this, SLOT(OnMatSliderChanged()));
	verify_connect(sliderMatSpecularB, SIGNAL(sliderMoved(int)), this, SLOT(OnMatSliderChanged()));
	verify_connect(chkMatSpecularSync, SIGNAL(clicked()), this, SLOT(OnMatSpecularSync()));
}

LightWindow::~LightWindow()
{
}

void LightWindow::OnCboLightIndexChanged(int)
{
	LoadSelectedLight();
}

void LightWindow::OnLightChanged()
{
	// Called when any light attribute is modified.

	if (mb_ChangeMask)
		return;

	Light* pLight = GetLight();
	if (pLight == nullptr)
		return;

	pLight->SetColor((float)spinLightR->value(), (float)spinLightG->value(), (float)spinLightB->value());

	GetApp()->GetMainWindow()->UpdateChildWindows();
}

void LightWindow::OnLightSliderChanged()
{
	// Called when any of the slider controls for the 
	// current light are modified.

	if (mb_ChangeMask)
		return;

	Light* pLight = GetLight();
	if (pLight == nullptr)
		return;

	if (chkLightSync->isChecked())
	{
		sliderLightG->setValue(sliderLightR->value());
		sliderLightB->setValue(sliderLightR->value());
	}
	spinLightR->setValue(sliderLightR->value() * 0.01);
	spinLightG->setValue(sliderLightG->value() * 0.01);
	spinLightB->setValue(sliderLightB->value() * 0.01);
}

void LightWindow::OnLightSync()
{
	// Sync color RGB values, effectively reducing
	// to a single intensity value.

	if (chkLightSync->isChecked())
	{
		sliderLightG->setValue(sliderLightR->value());
		sliderLightB->setValue(sliderLightR->value());
		sliderLightG->setEnabled(false);
		sliderLightB->setEnabled(false);
		OnLightSliderChanged();
	}
	else
	{
		sliderLightG->setEnabled(true);
		sliderLightB->setEnabled(true);
	}
}

void LightWindow::LoadSelectedLight()
{
	Light* pLight = GetLight();
	if (pLight == nullptr)
		return;

	mb_ChangeMask = true;

	char buf[32];
	snprintf(buf, sizeof(buf), "(%0.1f, %0.1f, %0.1f)", pLight->GetPos().x, pLight->GetPos().y, pLight->GetPos().z);
	labLightPos->setText(buf);

	spinLightR->setValue(pLight->GetRed());
	spinLightG->setValue(pLight->GetGreen());
	spinLightB->setValue(pLight->GetBlue());

	sliderLightR->setValue(pLight->GetRed() * 100.0);
	sliderLightG->setValue(pLight->GetGreen() * 100.0);
	sliderLightB->setValue(pLight->GetBlue() * 100.0);

	mb_ChangeMask = false;
}

void LightWindow::OnCboMaterialIndexChanged(int)
{
	LoadSelectedMaterial();
}

void LightWindow::OnMatChanged()
{
	// Called when any material attribute is modified.

	if (mb_ChangeMask)
		return;

	Material* pMat = GetMaterial();
	if (pMat == nullptr)
		return;

	pMat->SetAmbient(vec3(spinMatAmbientR->value(), spinMatAmbientG->value(), spinMatAmbientB->value()));
	pMat->SetDiffuse(vec3(spinMatDiffuseR->value(), spinMatDiffuseG->value(), spinMatDiffuseB->value()));
	pMat->SetSpecular(vec3(spinMatSpecularR->value(), spinMatSpecularG->value(), spinMatSpecularB->value()));
	pMat->SetShininess(spinMatShininess->value());

	GetApp()->GetMainWindow()->UpdateChildWindows();
}

void LightWindow::OnMatSliderChanged()
{
	// Called when any of the slider controls for the 
	// current material are modified.

	if (mb_ChangeMask)
		return;

	Material* pMat = GetMaterial();
	if (pMat == nullptr)
		return;

	if (chkMatAmbientSync->isChecked())
	{
		sliderMatAmbientG->setValue(sliderMatAmbientR->value());
		sliderMatAmbientB->setValue(sliderMatAmbientR->value());
	}
	spinMatAmbientR->setValue(sliderMatAmbientR->value() * 0.01);
	spinMatAmbientG->setValue(sliderMatAmbientG->value() * 0.01);
	spinMatAmbientB->setValue(sliderMatAmbientB->value() * 0.01);

	if (chkMatDiffuseSync->isChecked())
	{
		sliderMatDiffuseG->setValue(sliderMatDiffuseR->value());
		sliderMatDiffuseB->setValue(sliderMatDiffuseR->value());
	}
	spinMatDiffuseR->setValue(sliderMatDiffuseR->value() * 0.01);
	spinMatDiffuseG->setValue(sliderMatDiffuseG->value() * 0.01);
	spinMatDiffuseB->setValue(sliderMatDiffuseB->value() * 0.01);

	if (chkMatSpecularSync->isChecked())
	{
		sliderMatSpecularG->setValue(sliderMatSpecularR->value());
		sliderMatSpecularB->setValue(sliderMatSpecularR->value());
	}
	spinMatSpecularR->setValue(sliderMatSpecularR->value() * 0.01);
	spinMatSpecularG->setValue(sliderMatSpecularG->value() * 0.01);
	spinMatSpecularB->setValue(sliderMatSpecularB->value() * 0.01);
}

void LightWindow::OnMatAmbientSync()
{
	// Sync ambient RGB values, effectively reducing
	// to a single intensity value.

	if (chkMatAmbientSync->isChecked())
	{
		sliderMatAmbientG->setValue(sliderMatAmbientR->value());
		sliderMatAmbientB->setValue(sliderMatAmbientR->value());
		sliderMatAmbientG->setEnabled(false);
		sliderMatAmbientB->setEnabled(false);
		OnMatSliderChanged();
	}
	else
	{
		sliderMatAmbientG->setEnabled(true);
		sliderMatAmbientB->setEnabled(true);
	}
}

void LightWindow::OnMatDiffuseSync()
{
	// Sync diffuse RGB values, effectively reducing
	// to a single intensity value.

	if (chkMatDiffuseSync->isChecked())
	{
		sliderMatDiffuseG->setValue(sliderMatDiffuseR->value());
		sliderMatDiffuseB->setValue(sliderMatDiffuseR->value());
		sliderMatDiffuseG->setEnabled(false);
		sliderMatDiffuseB->setEnabled(false);
		OnMatSliderChanged();
	}
	else
	{
		sliderMatDiffuseG->setEnabled(true);
		sliderMatDiffuseB->setEnabled(true);
	}
}

void LightWindow::OnMatSpecularSync()
{
	// Sync specular RGB values, effectively reducing
	// to a single intensity value.

	if (chkMatSpecularSync->isChecked())
	{
		sliderMatSpecularG->setValue(sliderMatSpecularR->value());
		sliderMatSpecularB->setValue(sliderMatSpecularR->value());
		sliderMatSpecularG->setEnabled(false);
		sliderMatSpecularB->setEnabled(false);
		OnMatSliderChanged();
	}
	else
	{
		sliderMatSpecularG->setEnabled(true);
		sliderMatSpecularB->setEnabled(true);
	}
}

void LightWindow::LoadSelectedMaterial()
{
	Material* pMat = GetMaterial();
	if (pMat == nullptr)
		return;

	mb_ChangeMask = true;

	spinMatAmbientR->setValue(pMat->GetAmbient().r);
	spinMatAmbientG->setValue(pMat->GetAmbient().g);
	spinMatAmbientB->setValue(pMat->GetAmbient().b);

	sliderMatAmbientR->setValue(pMat->GetAmbient().r * 100.0);
	sliderMatAmbientG->setValue(pMat->GetAmbient().g * 100.0);
	sliderMatAmbientB->setValue(pMat->GetAmbient().b * 100.0);

	spinMatDiffuseR->setValue(pMat->GetDiffuse().r);
	spinMatDiffuseG->setValue(pMat->GetDiffuse().g);
	spinMatDiffuseB->setValue(pMat->GetDiffuse().b);

	sliderMatDiffuseR->setValue(pMat->GetDiffuse().r * 100.0);
	sliderMatDiffuseG->setValue(pMat->GetDiffuse().g * 100.0);
	sliderMatDiffuseB->setValue(pMat->GetDiffuse().b * 100.0);

	spinMatSpecularR->setValue(pMat->GetSpecular().r);
	spinMatSpecularG->setValue(pMat->GetSpecular().g);
	spinMatSpecularB->setValue(pMat->GetSpecular().b);

	sliderMatSpecularR->setValue(pMat->GetSpecular().r * 100.0);
	sliderMatSpecularG->setValue(pMat->GetSpecular().g * 100.0);
	sliderMatSpecularB->setValue(pMat->GetSpecular().b * 100.0);

	spinMatShininess->setValue(pMat->GetShininess());

	mb_ChangeMask = false;
}

Light* LightWindow::GetLight()
{
	// Get pointer to selected light.

	if (cboLight->currentIndex() < 0)
		return nullptr;

	int lightNumber = cboLight->currentIndex();
	return GLManager::GetLight(lightNumber);
}

Material* LightWindow::GetMaterial()
{
	// Get pointer to selected material.

	if (cboMaterial->currentIndex() < 0)
		return nullptr;

	XString matName = cboMaterial->currentText();
	return GLManager::GetMaterial(matName.c_str());
}

void LightWindow::SetDefaultTab(Tab tab)
{
	switch (tab)
	{
	case Tab::Light:
		tabWidget->setCurrentIndex(0);
		break;
	case Tab::Material:
		tabWidget->setCurrentIndex(1);
		break;
	default:
		tabWidget->setCurrentIndex(0);
		break;
	}
}

void LightWindow::OnClose()
{
	close();
}

void LightWindow::closeEvent(QCloseEvent* /* event */)
{
}
