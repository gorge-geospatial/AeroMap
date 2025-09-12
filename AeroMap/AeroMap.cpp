// AeroMap.cpp
// Application class
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include <assert.h>
#include <Shlobj.h>				// SHGetKnownFolderPath

#include <QMessageBox>

#include "ConfigDlg.h"
#include "MainWindow.h"
#include "AeroMap.h"			// application header

AeroMap::AeroMap(int& argc, char** argv)
	: QApplication(argc, argv)
	, mp_OutputWindow(nullptr)
	, ms_AppDataPath("")
	, m_ViewType(ViewType::None)
{
	setWindowIcon(QIcon(":/Images/AeroMap.ico"));

	LoadSettings();

	QDir dir(ms_AppDataPath.c_str());
	if (ms_AppDataPath.IsEmpty() || (dir.exists() == false))
	{
		QString str = tr("Invalid application data path: ");
		str += ms_AppDataPath.c_str();
		QMessageBox::information(nullptr, tr("AeroMap"), str, QMessageBox::StandardButton::Ok);
		ConfigDlg dlg;
		if (dlg.exec() == QDialog::Accepted)
		{
			LoadSettings();
		}
	}

	GDALAllRegister();

	Logger::Init(ms_AppDataPath, APP_NAME ".log");

	ms_ProgLibPath = ResolveProgLibPath(argc, argv);
	if (ms_ProgLibPath.GetLength() == 0)
	{
		Logger::Write(__FUNCTION__, "'ProgLib' path not found (from '%s')", argv[0]);
		QMessageBox::warning(nullptr, APP_NAME, "ProgLib folder not found. Photogrammetry process will not run.", QMessageBox::StandardButton::Ok);
	}

	CheckDependencies();
}

AeroMap::~AeroMap()
{
}

void AeroMap::LoadSettings()
{
	QSettings settings(ORG_KEY, APP_KEY);
	ms_AppDataPath = XString(settings.value(PATH_APPDATA_KEY, ms_AppDataPath.c_str()).toString());
}

void AeroMap::ActivateView(AeroMap::ViewType view, int subItem)
{
	m_ViewType = view;

	emit view_change_sig(view, subItem);
}

AeroMap::ViewType AeroMap::GetActiveView()
{
	return m_ViewType;
}

MainWindow* AeroMap::GetMainWindow()
{
	// Return ptr to one and only mainwindow instance.

	foreach (QWidget* pWidget, qApp->topLevelWidgets())
	{
		if (MainWindow* pMainWindow = dynamic_cast<MainWindow*>(pWidget))
			return pMainWindow;
	}
	return nullptr;
}

XString AeroMap::GetEnvValue(const char* envKey)
{
	// return a value from the environment
	//
	// this is not a MarkLib function because it uses Qt

	XString strValue;

	if (envKey)
	{
		// get current environment variables
		QStringList envList = QProcess::systemEnvironment();
		for (int i = 0; i < envList.size(); ++i)
		{
			XString kv = envList[i];
			int pos = kv.Find('=');
			if (pos > -1)
			{
				if (kv.Left(pos).CompareNoCase(envKey))
				{
					strValue = kv.Mid(pos + 1);
					break;
				}
			}
		}
	}

	return strValue;
}

void AeroMap::LogWrite(const char* text, ...)
{
	char buf[255] = { 0 };

	va_list ap;
	va_start(ap, text);
	vsprintf(buf, text, ap);
	va_end(ap);

	Logger::Write("", buf);

	// echo to output window
	if (mp_OutputWindow)
		mp_OutputWindow->AppendText(buf);
}

void AeroMap::SetComboByUserData(QComboBox* cbo, int userData)
{
	// Set qt combo box index by user data value.
	//

	int c = cbo->count();
	for (int i = 0; i < c; ++i)
	{
		int u = cbo->currentData().toInt();
		if (userData == u)
		{
			cbo->setCurrentIndex(i);
			break;
		}
	}
}

void AeroMap::DumpRaster(GDALDataset* pDS, const char* fileName)
{
	// Debugging function to dump the contents of a dataset.
	//
	// Inputs:
	//		pDS = gdal dataset w/1 raster band
	//		fileName = output text file name
	//

	assert(pDS);
	assert(fileName);

	int sizeX = pDS->GetRasterXSize();
	int sizeY = pDS->GetRasterYSize();
	int bandCount = pDS->GetRasterCount();
	if (bandCount != 1)
	{
		Logger::Write(__FUNCTION__, "Unexpected band count: %d.", bandCount);
		return;
	}

	GDALRasterBand* pBand = pDS->GetRasterBand(1);

	FILE* pFile = fopen(fileName, "wt");
	assert(pFile);

	// output srs
	if (pDS->GetSpatialRef() != nullptr)
	{
		char* pWkt = nullptr;
		OGRErr err = pDS->GetSpatialRef()->exportToPrettyWkt(&pWkt);
		if (err == OGRERR_NONE)
		{
			fprintf(pFile, "Spatial Reference System:\n");
			fprintf(pFile, pWkt);
		}

		CPLFree(pWkt);
	}

	// output geotransform
	double geoTransform[6] = {0};
	CPLErr err = pDS->GetGeoTransform(geoTransform);
	if (err == CPLErr::CE_None)
	{
		fprintf(pFile, "GeoTransform:\n");
		fprintf(pFile, "    [0] %f\n", geoTransform[0]);
		fprintf(pFile, "    [1] %f\n", geoTransform[1]);
		fprintf(pFile, "    [2] %f\n", geoTransform[2]);
		fprintf(pFile, "    [3] %f\n", geoTransform[3]);
		fprintf(pFile, "    [4] %f\n", geoTransform[4]);
		fprintf(pFile, "    [5] %f\n", geoTransform[5]);
		fprintf(pFile, "\n");
	}

	float* pScanline = (float *)CPLMalloc(sizeof(float)*sizeX);

	// read first row

	for (int y = 0; y < sizeY; ++y)
	{
		CPLErr err = pBand->RasterIO(
			GF_Read,			// read or write
			0,					// xoffset - zero based offset from left
			y,					// yoffset - zero based offset from top
			sizeX,				// width of the region of the band to be accessed in pixels
			1,					// height of the region of the band to be accessed in lines
			pScanline,			// data buffer
			sizeX,				// width of data buffer
			1,					// height of data buffer
			GDT_Float32,		// buffer type - data automatically translated
			0, 0);
		if (err == CPLErr::CE_None)
		{
			for (int x = 0; x < sizeX; ++x)
			{
				fprintf(pFile, "%f,", pScanline[x]);
			}
		}
		else
		{
			Logger::Write(__FUNCTION__, "GDALRasterBand::RasterIO() returned: %d", (int)err);
		}

		fprintf(pFile, "\n");
	}

	CPLFree(pScanline);

	fclose(pFile);
}

void AeroMap::DumpVector(GDALDataset* pDS, const char* fileName)
{
	// Debugging function to dump the contents of a dataset.
	//
	// Inputs:
	//		pDS = gdal dataset w/1 raster band
	//		fileName = output text file name
	//

	assert(pDS);
	assert(fileName);

	FILE* pFile = fopen(fileName, "wt");
	assert(pFile);

	fprintf(pFile, "Filename: %s\n", fileName);
	fprintf(pFile, "Layer Count: %d\n", pDS->GetLayerCount());

	int layerCount = pDS->GetLayerCount();
	for (int layer = 0; layer < layerCount; ++layer)
	{
		fprintf(pFile, "    Layer %d\n", layer);
		OGRLayer* pLayer = pDS->GetLayer(layer);
		fprintf(pFile, "        Name: %s\n", pLayer->GetName());
		fprintf(pFile, "        Desc: %s\n", pLayer->GetDescription());
		
		fprintf(pFile, "        Feature Count: %lld\n", pLayer->GetFeatureCount());
		
		pLayer->ResetReading();
		OGRFeature* pFeature;
		while ((pFeature = pLayer->GetNextFeature()) != nullptr)
		{
			OGRFeatureDefn* pFeatureDef = pFeature->GetDefnRef();
			fprintf(pFile, "            Feature Name: %s\n", pFeatureDef->GetName());
			OGRwkbGeometryType geom = pFeatureDef->GetGeomType();
			fprintf(pFile, "            Geometry: %s\n", OGRGeometryTypeToName(geom));

			int fieldCount = pFeatureDef->GetFieldCount();
			fprintf(pFile, "            Field Count: %d\n", fieldCount);
		
			fprintf(pFile, "            Fields:\n");
			fprintf(pFile, "                ");
			for (int field = 0; field < fieldCount; ++field)
			{
				OGRFieldDefn* pFieldDef = pFeatureDef->GetFieldDefn(field);

				switch (pFieldDef->GetType())
				{
				case OFTInteger:
					fprintf(pFile, "%d,", pFeature->GetFieldAsInteger(field));
					break;
				case OFTInteger64:
					fprintf(pFile, CPL_FRMT_GIB ",", pFeature->GetFieldAsInteger64(field));
					break;
				case OFTReal:
					fprintf(pFile, "%.3f,", pFeature->GetFieldAsDouble(field));
					break;
				case OFTString:
					fprintf(pFile, "%s,", pFeature->GetFieldAsString(field));
					break;
				default:
					fprintf(pFile, "%s,", pFeature->GetFieldAsString(field));
					break;
				}
			}
			fprintf(pFile, "\n");

			int geomFieldCount = pFeature->GetGeomFieldCount();
			fprintf(pFile, "            Geometry Field Count: %d\n", geomFieldCount);

			fprintf(pFile, "            Geometry Fields:\n");
			for (int geomField = 0; geomField < geomFieldCount; ++geomField)
			{
				OGRGeometry* pGeometry = pFeature->GetGeomFieldRef(geomField);
				if (pGeometry != nullptr)
				{
					OGRwkbGeometryType geomType = wkbFlatten(pGeometry->getGeometryType());
					switch (geomType) {
					case wkbPoint:
						{
							OGRPoint* pPoint = pGeometry->toPoint();
							fprintf(pFile, "                Point: %.3f,%3.f\n", pPoint->getX(), pPoint->getY());
							break;
						}
					case wkbLineString:
						{
							OGRLineString* pLineString = pGeometry->toLineString();
							int pointCount = pLineString->getNumPoints();
							fprintf(pFile, "                Point Count: %d\n", pointCount);
							fprintf(pFile, "                Points:  ");
							for (int pt = 0; pt < pointCount; ++pt)
							{
								OGRPoint point;
								pLineString->getPoint(pt, &point);
								fprintf(pFile, "(%.3f,%3.f) ", point.getX(), point.getY());
							}
							fprintf(pFile, "\n");
							break;
						}
					case wkbPolygon:
						{
							OGRPolygon* pPoly = pGeometry->toPolygon();
							int ringCount = pPoly->getNumInteriorRings();
							fprintf(pFile, "                Inner Ring Count: %d\n", ringCount);
							fprintf(pFile, "                Inner Rings:  ");
							for (int ri = 0; ri < ringCount; ++ri)
							{
								OGRLinearRing* pRing = pPoly->getInteriorRing(ri);
								int ringPointCount = pRing->getNumPoints();
								for (int i = 0; i < ringPointCount; ++i)
								{
									OGRPoint point;
									pRing->getPoint(i, &point);
									fprintf(pFile, "(%.3f,%3.f) ", point.getX(), point.getY());
								}
							}
							fprintf(pFile, "\n");
							break;
						}
					default:
						fprintf(pFile, "Unhandled geometry type: %d\n", (int)geomType);
						assert(false);
					}
				}
			}
			fprintf(pFile, "\n");
		}
		//not sure 
		//OGRFeature::DestroyFeature(pFeature);

		// output srs
		if (pLayer->GetSpatialRef() != nullptr)
		{
			fprintf(pFile, "Spatial Reference System:\n");
			int epsg = pLayer->GetSpatialRef()->GetEPSGGeogCS();
			fprintf(pFile, "    EPSG: %d\n", epsg);
			fprintf(pFile, "    WKT:\n");
			char* pWkt = nullptr;
			OGRErr err = pLayer->GetSpatialRef()->exportToPrettyWkt(&pWkt);
			if (err = CPLErr::CE_None)
				fprintf(pFile, pWkt);
			else
				fprintf(pFile, "    Unable to format SRS as WKT.");
			CPLFree(pWkt);
		}
	}

	fclose(pFile);
}

XString AeroMap::ResolveAppDataPath()
{
	// Jump thru some hoops to get app data path. Append app name
	// to get something like: C:\Users\mark\AppData\Local\Ide51.
	//

	XString appDataPath;

	wchar_t* filepath = nullptr;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &filepath);
	if (hr == S_OK)
	{
		Logger::Write(__FUNCTION__, "SHGetKnownFolderPath(FOLDERID_LocalAppData) returned: %d", (int)hr);
	}

	// Convert the wchar_t string to a char* string. Record
	// the length of the original string and add 1 to it to
	// account for the terminating null character.
	size_t origsize = wcslen(filepath) + 1;
	size_t convertedChars = 0;

	// Allocate two bytes in the multibyte output string for every wide
	// character in the input string (including a wide character
	// null). Because a multibyte character can be one or two bytes,
	// you should allot two bytes for each character. Having extra
	// space for the new string is not an error, but having
	// insufficient space is a potential security problem.
	const size_t newsize = (origsize + 1) * 2;
	// The new string will contain a converted copy of the original
	// string plus the type of string appended to it.
	char* nstring = new char[newsize];

	// Put a copy of the converted string into nstring
	wcstombs_s(&convertedChars, nstring, newsize, filepath, _TRUNCATE);

	appDataPath = nstring;

	// append appname
	appDataPath = XString::CombinePath(appDataPath.c_str(), APP_NAME);

	// dealloc filePath - have no way of confirming, but saw many 
	// examples that did it this way
	CoTaskMemFree(filepath);

	// path, with appname, may not exists
	QDir dir;
	if (dir.exists(appDataPath.c_str()) == false)
		dir.mkpath(appDataPath.c_str());

	return appDataPath;
}

XString AeroMap::ResolveProgLibPath(int& argc, char** argv)
{
	// Starting at location of exe, traverse up until "ProgLib"
	// found; this ok for dev, but an install may need different
	// approach

	XString lib_path;

	if (argc > 0)
	{
		XString cur_path = XString(argv[0]).GetPathName();
		
		QDir dir(cur_path.c_str());
		while (dir.isRoot() == false)
		{
			XString path = XString::CombinePath(cur_path, "ProgLib");
			if (QFile::exists(path.c_str()))
			{
				lib_path = path;
				break;
			}
			dir.cdUp();
			cur_path = dir.absolutePath();
		}
	}

	return lib_path;
}

void AeroMap::CheckDependencies()
{
	// OpenCV 4.10 or later required
	if ((cv::getVersionMajor() < 4) || (cv::getVersionMajor() == 4) && (cv::getVersionMinor() < 10))
	{
		XString str = XString::Format("OpenCV version 4.10 required, detected %d.%d", cv::getVersionMajor(), cv::getVersionMinor());
		Logger::Write(__FUNCTION__, str.c_str());
		QMessageBox::warning(nullptr, APP_NAME, str.c_str(), QMessageBox::StandardButton::Ok);
	}
}
