#ifndef SVGFILE_H
#define SVGFILE_H

#include "Calc.h"
#include "TinyXML2.h"
#include "XString.h"

class SvgFile
{
public:

	enum class ElemType
	{
		None,
		Rectangle,	// <rect>
		Circle,		// <circle>
		Ellipse,	// <ellipse>
		Line,		// <line>
		PolyLine,	// <polyline>
		Polygon,	// <polygon>
		Path,		// <path>
		Text,		// <text>
	};

	enum class PathCmd
	{
		None,
		M,	// M = moveto, absolute
		m,	// M = moveto, relative
		L,	// L = lineto
		l,
		H,	// H = horizontal lineto
		h,
		V,	// V = vertical lineto
		v,
		C,	// C = curveto
		c,
		S,	// S = smooth curveto
		s,
		Q,	// Q = quadratic Bézier curve
		q,
		T,	// T = smooth quadratic Bézier curveto
		t,
		A,	// A = elliptical Arc
		a,
		Z,	// Z = closepath
		z
	};

	struct PathEntry
	{
		PathCmd cmd;
		double x, y;
		bool IsSelected;		// "path segment 0 selected" means segment from point 0 to 1

		PathEntry()
			: cmd(PathCmd::None)
			, IsSelected(false)
			, x(0.0)
			, y(0.0)
		{
		}
	};

	struct ElemDesc
	{
		ElemType type;
		XString id;
		double x, y;
		double w, h;
		RectD ext;				// extents, world units
		XString style;			// style string
		bool IsSelected;
		bool IsVisible;
		std::vector<PathEntry> path;		// list of points used for path, polygon, polyline, etc.
		bool IsPathClosed;		// true => connect last point to first

		ElemDesc()
			: type(ElemType::None)
			, x(0.0), y(0.0)
			, w(0.0), h(0.0)
			, IsSelected(false)
			, IsVisible(true)
			, IsPathClosed(false)
		{
		}
	};

public:

	SvgFile(const char* fileName = nullptr);
	~SvgFile();

	bool Load(const char* fileName);
	bool Save(const char* fileName = nullptr);
	bool Export(const char* fileName, bool visible = true);
	XString GetFileName();
	bool IsDirty();

	int  GetElementCount() { return static_cast<int>(m_elemList.size()); }
	ElemDesc* GetElement(int index) { return &m_elemList[index]; }
	int  GetSelectedElementCount();
	int  GetSelectedElementIndex();
	int  GetVisibleElementCount();
	void SetElementVisible(int index, bool render);
	bool IsElementVisible(int index);
	void SelectPathSegment(int elemIndex, int segIndex);
	bool IsPathSegmentSelected(int elemIndex, int segIndex);
	bool FindPathSegment(PointD pt0, PointD pt1, double epsilon, int& elemIndex, int& segIndex, int skipElem = -1);

	void AddPolyLine(std::vector<PointD>& ptList);

	int  Thin(int percent, bool selected);

	void Scale(double sf);
	void ScaleX(double sf);
	void ScaleY(double sf);

	void FlipVert();
	void FlipHorz();
	void Rotate90R();
	void Rotate90L();
	void Rotate180();

	void Center();
	void Move(double dx, double dy);
	void TranslatePosXY();

	void DeselectAll(bool deep = true);
	void SelectAtPoint(double x, double y, bool deselect = true);
	void SelectInRect(double x0, double y0, double x1, double y1, bool deselect = true);

	RectD GetExtents();
	void  SetMousePos(double x, double y);
	void  SetPixelSize(double pixelSize);

	void  Delete();
	void  DeleteSubRange(int elemIndex, int segStart, int segEnd, XString name1, XString name2);
	int   MergePaths(double epsilon);

	static const char* ElemTypeToText(ElemType elemType);
	static const char* PathCmdToText(PathCmd cmd);

private:

	std::vector<ElemDesc> m_elemList;		// svg elements

	PointD m_ptMouse;			// mouse cursor location, world units
	PointD m_addAnchor;			// anchor point for element being added

	XString ms_FileName;
	bool mb_IsDirty;

	double mf_PixelSize;		// size of pixel in x and y direction

private:

	void LoadGroup(TinyXML2::XMLElement* pElem);
	void LoadLine(TinyXML2::XMLElement* pElem);
	void LoadPath(TinyXML2::XMLElement* pElem, MAT3* pmat = nullptr);
	void LoadPolygon(TinyXML2::XMLElement* pElem);
	void LoadPolyLine(TinyXML2::XMLElement* pElem);
	void LoadRect(TinyXML2::XMLElement* pElem);

	bool IsGroup(TinyXML2::XMLElement* pElem);
	bool IsPath(TinyXML2::XMLElement* pElem);
	bool IsRect(TinyXML2::XMLElement* pElem);
	bool IsPoly(TinyXML2::XMLElement* pElem);
	bool IsPolyLine(TinyXML2::XMLElement* pElem);
	bool IsLine(TinyXML2::XMLElement* pElem);
	bool IsMeta(TinyXML2::XMLElement* pElem);
	bool IsDefs(TinyXML2::XMLElement* pElem);
	bool IsClipPath(TinyXML2::XMLElement* pElem);
	bool IsTitle(TinyXML2::XMLElement* pElem);
	bool IsDesc(TinyXML2::XMLElement* pElem);

	PathCmd CharToPathCmd(char c);
	void UpdateExtents(ElemDesc& el);
	bool CloseToLine(PointD pt, RectD line);
	MAT3 TransformToMatrix(XString strTransformEntry);
	PointD TransformToDelta(XString strTransform);
};

#endif // #ifndef SVGFILE_H
