#ifndef STAGEREPORT_H
#define STAGEREPORT_H

#include "Stage.h"

class StageReport : Stage
{
public:

	virtual int Run() override;

	StageReport(DroneProc* pDroneProc);
	~StageReport();

private:

	QFont m_FontHeader;
	QFont m_FontHeader1;
	QFont m_FontBody;

	QPdfWriter* mp_Writer;
	QPainter* mp_Painter;

	int m_dpi;
	int m_col0;
	int m_col1;

	std::string m_proc_date;
	std::string m_start_date;
	std::string m_end_date;
	double m_area;

private:

	void SetupFonts();
	void LoadStatistics();

	void OutputSummary();
	void OutputOrthophoto();
	void OutputSFM();
	void OutputParameters();
	void OutputHeader(bool new_page = true);
	void OutputText(int x, int y, const char* text, ...);
};

#endif // #ifndef STAGEREPORT_H
