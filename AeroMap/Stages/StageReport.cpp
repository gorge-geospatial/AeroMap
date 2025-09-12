// StageReport.cpp
// Generate PDF report.
//

#include <fstream>
#include <nlohmann/json.h>
using json = nlohmann::json; 

#include <QPdfWriter>

#include "StageReport.h"

const int OFFSETV = 240;		// default line spacing

int StageReport::Run()
{
	// Inputs:
	// Outputs:
	//		+ report
	//			report.pdf
	//

	int status = 0;

	GetApp()->LogWrite("Report...");
	BenchmarkStart();
	LogStage();

	AeroLib::CreateFolder(tree.report_path);

	XString file_name = XString::CombinePath(tree.report_path, "report.pdf");
	QFile outputFile(file_name.c_str());
	if (!outputFile.open(QIODevice::WriteOnly))
	{
		Logger::Write(__FUNCTION__, "Unable to create '%s'", file_name.c_str());
		assert(false);
		return -1;
	}

	mp_Writer = new QPdfWriter(&outputFile);
	mp_Writer->setPageSize(QPageSize(QPageSize::Letter));
	mp_Writer->setPageOrientation(QPageLayout::Portrait);
	m_dpi = mp_Writer->resolution();
	m_col0 = m_dpi / 2;
	m_col1 = m_dpi;

	mp_Painter = new QPainter(mp_Writer);

	SetupFonts();
	LoadStatistics();

	OutputSummary();
	OutputSFM();
	OutputOrthophoto();
	OutputParameters();

	delete mp_Painter;

	outputFile.close();
	delete mp_Writer;

	GetProject().Update();		// trigger ProjectWindow update
	BenchmarkStop();

	return status;
}

void StageReport::OutputSummary()
{
	OutputHeader(false);

	mp_Painter->setFont(m_FontHeader1);
	mp_Painter->drawText(m_col0, 1000, "Summary");

	mp_Painter->setFont(m_FontBody);
	int y = 1400;
	OutputText(m_col1, y += OFFSETV, "Date Processed: %s", m_proc_date.c_str());
	OutputText(m_col1, y += OFFSETV, "Date Collected: %s", m_end_date.c_str());
	OutputText(m_col1, y += OFFSETV, "Area Covered: %0.3f sq m", m_area);
	OutputText(m_col1, y += OFFSETV, "Images: %d", GetProject().GetImageCount());
}

void StageReport::OutputSFM()
{
	XString file_name = XString::CombinePath(tree.opensfm, "stats/topview.png");
	if (AeroLib::FileExists(file_name) == false)
		return;

	QImage image;
	if (image.load(file_name.c_str()) == false)
	{
		Logger::Write(__FUNCTION__, "Unable to load: '%s'", file_name.c_str());
		return;
	}

	OutputHeader();

	mp_Painter->setFont(m_FontHeader1);
	mp_Painter->drawText(m_col0, 1000, "OpenSFM");

	// desired size of image, inches
	const double WIDTH_IN = 6.5;
	double height_in = ((double)image.height() / (double)image.width()) * WIDTH_IN;

	QRect rectTarget(1000, 1600, m_dpi * WIDTH_IN, m_dpi * height_in);
	QRect rectSrc = image.rect();
	mp_Painter->drawImage(rectTarget, image, rectSrc, Qt::AutoColor);
}

void StageReport::OutputOrthophoto()
{
	if (AeroLib::FileExists(tree.orthophoto_tif) == false)
		return;

	QImage image;
	if (image.load(tree.orthophoto_tif.c_str()) == false)
	{
		Logger::Write(__FUNCTION__, "Unable to load orthophoto: '%s'", tree.orthophoto_tif.c_str());
		return;
	}

	int MAX_IMAGE_SIZE = 800;
	int w = image.width();
	int h = image.height();
	int max_dim = (w > h ? w : h);
	if (max_dim > MAX_IMAGE_SIZE)
	{
		// since image is stored in pdf - and loaded every time
		// report is viewed - scale down to a reasonable size
		double sf = (double)MAX_IMAGE_SIZE / (double)max_dim;
		image = image.scaled(w * sf, h * sf, Qt::AspectRatioMode::KeepAspectRatio);
	}

	XString ortho_file = XString::CombinePath(tree.report_path, "ortho.png");
	if (image.save(ortho_file.c_str(), "PNG") == false)
	{
		Logger::Write(__FUNCTION__, "Unable to write orthophoto: '%s'", ortho_file.c_str());
		return;
	}

	OutputHeader();

	mp_Painter->setFont(m_FontHeader1);
	mp_Painter->drawText(m_col0, 1000, "Orthophoto");

	// desired size of image, inches
	const double WIDTH_IN = 6.5;
	double height_in = ((double)image.height() / (double)image.width()) * WIDTH_IN;

	QRect rectTarget(1000, 1600, m_dpi * WIDTH_IN, m_dpi * height_in);
	QRect rectSrc = image.rect();
	mp_Painter->drawImage(rectTarget, image, rectSrc, Qt::AutoColor);
}

void StageReport::OutputParameters()
{
	OutputHeader();

	mp_Painter->setFont(m_FontHeader1);
	mp_Painter->drawText(m_col0, 1000, "Parameters");

	mp_Painter->setFont(m_FontBody);
	int y = 1400;
	OutputText(m_col1, y += OFFSETV, "DEM Resolution: %0.1f", arg.dem_resolution);
	OutputText(m_col1, y += OFFSETV, "DEM Gap Fill Steps: %0.1f", arg.dem_gapfill_steps);
	OutputText(m_col1, y += OFFSETV, "DEM Decimation: %d", arg.dem_decimation);
	OutputText(m_col1, y += OFFSETV, "Orthophoto Resolution: %0.1f", arg.ortho_resolution);
	OutputText(m_col1, y += OFFSETV, "Crop Buffer: %0.1f", arg.crop);
	OutputText(m_col1, y += OFFSETV, "Fast Orthophoto: %s", arg.fast_orthophoto ? "True" : "False");
	OutputText(m_col1, y += OFFSETV, "Point Cloud Quality: %s", arg.pc_quality.c_str());
	OutputText(m_col1, y += OFFSETV, "Point Cloud Filter: %0.2f", arg.pc_filter);
	OutputText(m_col1, y += OFFSETV, "Point Cloud Sample: %0.2f", arg.pc_sample);
	OutputText(m_col1, y += OFFSETV, "DSM: %s", arg.dsm ? "True" : "False");
	OutputText(m_col1, y += OFFSETV, "DTM: %s", arg.dtm ? "True" : "False");
}

void StageReport::OutputHeader(bool new_page)
{
	if (new_page)
		mp_Writer->newPage();

	mp_Painter->setFont(m_FontHeader);
	mp_Painter->drawText(m_col0, m_dpi / 2, "AeroMap Report");
}

void StageReport::OutputText(int x, int y, const char* text, ...)
{
	char buf[255] = { 0 };

	va_list ap;
	va_start(ap, text);
	vsprintf(buf, text, ap);
	va_end(ap);

	mp_Painter->drawText(x, y, buf);
}

void StageReport::SetupFonts()
{
	m_FontHeader.setFamily("Cambria");
	m_FontHeader.setPointSize(12.0);
	m_FontHeader.setBold(true);
	m_FontHeader.setItalic(false);

	m_FontHeader1.setFamily("Cambria");
	m_FontHeader1.setPointSize(11.0);
	m_FontHeader1.setBold(true);
	m_FontHeader1.setItalic(false);

	m_FontBody.setFamily("Consolas");
	m_FontBody.setPointSize(10.0);
	m_FontBody.setBold(false);
	m_FontBody.setItalic(false);
}

void StageReport::LoadStatistics()
{
	// Load OpenSFM statistics.
	//

	XString file_name = XString::CombinePath(tree.opensfm, "stats/stats.json");

	std::ifstream f(file_name.c_str());
	json data = json::parse(f);

	if (data.contains("processing_statistics"))
	{
		json proc_stat = data["processing_statistics"];
		m_proc_date = proc_stat["date"].template get<std::string>();
		m_start_date = proc_stat["start_date"].template get<std::string>();
		m_end_date = proc_stat["end_date"].template get<std::string>();
		m_area = proc_stat["area"].template get<double>();
	}
}

StageReport::StageReport(DroneProc* pDroneProc)
	: Stage(pDroneProc)
{
	m_stage_id = Id::Report;
}

StageReport::~StageReport()
{

}
