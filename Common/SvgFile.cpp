// SvgFile.cpp
// Manager for SVG files.
//
// Although, I can imagine one day being able to edit
// these with some advanced tools, the immediate need 
// is to simply scale, crop or delete existing elements.
//
// Groups are not maintained after loading. On load, 
// any transform is applied to the children and no
// reference to the original group element is retained.
//

#include <assert.h>

#include "MarkLib.h"
#include "Logger.h"
#include "SvgFile.h"

const int PIXELS_CLOSE = 10;

SvgFile::SvgFile(const char* fileName)
	: mf_PixelSize(-1.0)
	, mb_IsDirty(false)
{
	if (fileName != nullptr)
		Load(fileName);
}

SvgFile::~SvgFile()
{
	m_elemList.clear();
}

int SvgFile::Thin(int stride, bool selected)
{
	// Reduce point count in selected path(s).
	//
	// "Path" includes path, polyline, polygon element
	// types.
	//
	// Inputs:
	//		stride = removed every [stride] point
	//		selected = only process selected paths, else all
	// Outputs:
	//		return = total # of points removed
	//

	if (stride < 1)
		return 0;

	int point_count = 0;

	for (int i = 0; i < m_elemList.size(); ++i)
	{
		switch (m_elemList[i].type) {
		case ElemType::Path:
		case ElemType::PolyLine:
		case ElemType::Polygon:
			if ((m_elemList[i].IsSelected) || (selected == false))
			{
				for (int pi = (int)m_elemList[i].path.size() - 1; pi >= 0; pi -= stride)
				{
					m_elemList[i].path.erase(m_elemList[i].path.begin() + pi);
					++point_count;
				}

				int path_size = (int)m_elemList[i].path.size();
				printf("path size = %d", path_size);
			}
			break;
		default:
			assert(false);
			break;
		}
	}

	mb_IsDirty = true;

	return point_count;
}

void SvgFile::Scale(double sf)
{
	// Scale equally along x & y axes.
	//

	for (int i = 0; i < m_elemList.size(); ++i)
	{
		switch (m_elemList[i].type)	{
		case ElemType::Path:
			{
				for (int pi = 0; pi < m_elemList[i].path.size(); ++pi)
				{
					m_elemList[i].path[pi].x *= sf;
					m_elemList[i].path[pi].y *= sf;
				}
			}
			break;
		case ElemType::Line:
			m_elemList[i].x *= sf;
			m_elemList[i].y *= sf;
			m_elemList[i].w *= sf;
			m_elemList[i].h *= sf;
			break;
		case ElemType::Rectangle:
			m_elemList[i].x *= sf;
			m_elemList[i].y *= sf;
			m_elemList[i].w *= sf;
			m_elemList[i].h *= sf;
			break;
		default:
			assert(false);
			break;
		}
		UpdateExtents(m_elemList[i]);
	}

	mb_IsDirty = true;
}

void SvgFile::ScaleX(double sf)
{
	// Scale along x axis.
	//

	for (int i = 0; i < m_elemList.size(); ++i)
	{
		switch (m_elemList[i].type)
		{
		case ElemType::Path:
			{
				for (int pi = 0; pi < m_elemList[i].path.size(); ++pi)
				{
					m_elemList[i].path[pi].x *= sf;
				}
			}
			break;
		case ElemType::Line:
			m_elemList[i].x *= sf;
			m_elemList[i].w *= sf;
			break;
		case ElemType::Rectangle:
			m_elemList[i].x *= sf;
			m_elemList[i].w *= sf;
			break;
		default:
			assert(false);
			break;
		}
		UpdateExtents(m_elemList[i]);
	}

	mb_IsDirty = true;
}

void SvgFile::ScaleY(double sf)
{
	// Scale along y axis.
	//

	for (int i = 0; i < m_elemList.size(); ++i)
	{
		switch (m_elemList[i].type)
		{
		case ElemType::Path:
			{
				for (int pi = 0; pi < m_elemList[i].path.size(); ++pi)
				{
					m_elemList[i].path[pi].y *= sf;
				}
			}
			break;
		case ElemType::Line:
			m_elemList[i].y *= sf;
			m_elemList[i].h *= sf;
			break;
		case ElemType::Rectangle:
			m_elemList[i].y *= sf;
			m_elemList[i].h *= sf;
			break;
		default:
			assert(false);
			break;
		}
		UpdateExtents(m_elemList[i]);
	}

	mb_IsDirty = true;
}

void SvgFile::FlipVert()
{
	for (int i = 0; i < m_elemList.size(); ++i)
	{
		switch (m_elemList[i].type)
		{
		case ElemType::Path:
			{
				for (int pi = 0; pi < m_elemList[i].path.size(); ++pi)
				{
					m_elemList[i].path[pi].y = -m_elemList[i].path[pi].y;
				}
			}
			break;
		case ElemType::Line:
				{
				}
				break;
		case ElemType::Rectangle:
				{
				}
				break;
		default:
			assert(false);
			break;
		}
		UpdateExtents(m_elemList[i]);
	}

	mb_IsDirty = true;
}

void SvgFile::FlipHorz()
{
	for (int i = 0; i < m_elemList.size(); ++i)
	{
		switch (m_elemList[i].type)
		{
		case ElemType::Path:
			{
				for (int pi = 0; pi < m_elemList[i].path.size(); ++pi)
				{
					m_elemList[i].path[pi].x = -m_elemList[i].path[pi].x;
				}
			}
			break;
		case ElemType::Line:
				{
				}
				break;
		case ElemType::Rectangle:
				{
				}
				break;
		default:
			assert(false);
			break;
		}
		UpdateExtents(m_elemList[i]);
	}

	mb_IsDirty = true;
}

void SvgFile::Rotate90R()
{
	for (int i = 0; i < m_elemList.size(); ++i)
	{
		switch (m_elemList[i].type)
		{
		case ElemType::Path:
			{
				for (int pi = 0; pi < m_elemList[i].path.size(); ++pi)
				{
					double x = m_elemList[i].path[pi].x;
					double y = m_elemList[i].path[pi].y;
					m_elemList[i].path[pi].x = y;
					m_elemList[i].path[pi].y = -x;
				}
			}
			break;
		case ElemType::Line:
				{
				}
				break;
		case ElemType::Rectangle:
				{
				}
				break;
		default:
			assert(false);
			break;
		}
		UpdateExtents(m_elemList[i]);
	}

	mb_IsDirty = true;
}

void SvgFile::Rotate90L()
{
	for (int i = 0; i < m_elemList.size(); ++i)
	{
		switch (m_elemList[i].type)
		{
		case ElemType::Path:
			{
				for (int pi = 0; pi < m_elemList[i].path.size(); ++pi)
				{
					double x = m_elemList[i].path[pi].x;
					double y = m_elemList[i].path[pi].y;
					m_elemList[i].path[pi].x = -y;
					m_elemList[i].path[pi].y = x;
				}
			}
			break;
		case ElemType::Line:
				{
				}
				break;
		case ElemType::Rectangle:
				{
				}
				break;
		default:
			assert(false);
			break;
		}
		UpdateExtents(m_elemList[i]);
	}

	mb_IsDirty = true;
}

void SvgFile::Rotate180()
{

}

void SvgFile::Move(double dx, double dy)
{
	// Move selected items.
	//
	// Inputs:
	//		dx = distance in x direction, world units
	//		dy = distance in y direction, world units
	//

	for (int i = 0; i < m_elemList.size(); ++i)
	{
		switch (m_elemList[i].type)
		{
		case ElemType::Path:
			// only move the init offset, others assumed to be relative
			if (m_elemList[i].path.size() > 0)
			{
				m_elemList[i].path[0].x += dx;
				m_elemList[i].path[0].y += dy;
			}
			break;
		case ElemType::Line:
				m_elemList[i].x += dx;
				m_elemList[i].y += dy;
				break;
		case ElemType::Rectangle:
				m_elemList[i].x += dx;
				m_elemList[i].y += dy;
				break;
		default:
			assert(false);
			break;
		}
		UpdateExtents(m_elemList[i]);
	}

	mb_IsDirty = true;
}

void SvgFile::Center()
{
	// Center about origin.
	//

	RectD ext = GetExtents();

	double dx = (ext.DX() * 0.5) + ext.x0;
	double dy = (ext.DY() * 0.5) + ext.y0;

	Move(-dx, -dy);
}

void SvgFile::TranslatePosXY()
{
	// Translate to +XY quadrant.
	//

	RectD ext = GetExtents();

	double dx = -ext.x0;
	double dy = -ext.y0;

	Move(dx, dy);
}

bool SvgFile::Load(const char* fileName)
{
	// Load SVG file.
	//

	m_elemList.clear();

	TinyXML2::XMLDocument doc;
	TinyXML2::XMLError status = doc.LoadFile(fileName);
	if (status != TinyXML2::XMLError::XML_SUCCESS)
	{
		Logger::Write(__FUNCTION__, "Error loading XML file: %d", static_cast<int>(status));
		return false;
	}

	// The Node object is the primary data type for the entire DOM.
	//
	// A node can be an element node, an attribute node, a text node, 
	// or any other of the node types explained in the "Node types" chapter.
	//
	// An XML element is everything from (including) the element's start 
	// tag to (including) the element's end tag.
	//
    // Rectangle <rect>
    // Circle <circle>
    // Ellipse <ellipse>
    // Line <line>
    // Polyline <polyline>
    // Polygon <polygon>
    // Path <path>
	//

	XString strRoot = "svg";

	TinyXML2::XMLElement* pRoot = doc.RootElement();
	assert(strRoot.CompareNoCase(pRoot->Name()));
	TinyXML2::XMLElement* pElem = pRoot->FirstChildElement();
	while (pElem != nullptr)
	{
		XString str = pElem->Value();
		if (IsGroup(pElem))
			LoadGroup(pElem);
		else if (IsPath(pElem))
			LoadPath(pElem);
		else if (IsRect(pElem))
			LoadRect(pElem);
		else if (IsLine(pElem))
			LoadLine(pElem);
		else if (IsPoly(pElem))
			LoadPolygon(pElem);
		else if (IsPolyLine(pElem))
			LoadPolyLine(pElem);
		else if (IsMeta(pElem))
			;
		else if (IsDefs(pElem))
			;
		else if (IsClipPath(pElem))
			;
		else if (IsTitle(pElem))
			;
		else if (IsDesc(pElem))
			;
		else if (str.BeginsWithNoCase("sodipodi:"))
			;
		else
		{
			Logger::Write(__FUNCTION__, "Unhandled element: %s", str.c_str());
			assert(false);
		}
		pElem = pElem->NextSiblingElement();
	}

	ms_FileName = fileName;
	mb_IsDirty = false;

	return true;
}

void SvgFile::LoadGroup(TinyXML2::XMLElement* pElemGroup)
{
	// Load group.
	//
	//<g style="fill:#b9b9b9;fill-opacity:1;stroke:#ffffff;stroke-width:103;stroke-linejoin:round;stroke-opacity:1" id="Clatsop" transform="translate(51.504435,51.502611)">

	XString strGroup = "g";

	assert(strGroup.CompareNoCase(pElemGroup->Name()));
	Logger::Write(__FUNCTION__, "Loading group.");

	bool transform = false;
	XString strTransform = pElemGroup->Attribute("transform");
	MAT3 mat;
	if (strTransform.IsEmpty() == false)
	{
		//matrix()
		//translate()
		//rotate()
		//scale()
		//skewX
		//skewY
		//transform = "rotate(-10 50 100)
			//translate(-36 45.5)
			//skewX(40)
			//scale(1 0.5)">


		Logger::Write(__FUNCTION__, "    Transform = %s.", strTransform.c_str());
		mat = TransformToMatrix(strTransform);
		transform = true;
	}

	TinyXML2::XMLElement* pElem = pElemGroup->FirstChildElement();
	while (pElem != nullptr)
	{
		XString str = pElem->Value();
		if (IsGroup(pElem))
			LoadGroup(pElem);
		else if (IsPath(pElem))
			LoadPath(pElem, transform ? &mat : nullptr);
		else if (IsRect(pElem))
			LoadRect(pElem);
		else if (IsLine(pElem))
			LoadLine(pElem);
		else if (IsPoly(pElem))
			LoadPolygon(pElem);
		else if (IsPolyLine(pElem))
			LoadPolyLine(pElem);
		else if (str.BeginsWithNoCase("sodipodi:"))
			;
		else
		{
			Logger::Write(__FUNCTION__, "Unhandled element: %s", str.c_str());
			assert(false);
		}
		pElem = pElem->NextSiblingElement();
	}
}

void SvgFile::LoadRect(TinyXML2::XMLElement* pElem)
{
	// Load SVG <rect> element.
	//

	ElemDesc ed;
	ed.type = ElemType::Rectangle;
	ed.id = pElem->Attribute("id");
	ed.style = pElem->Attribute("style");

	// must have height/width, x/y optional
	ed.w = atof(pElem->Attribute("width"));
	ed.h = atof(pElem->Attribute("height"));
	if (pElem->Attribute("x"))
		ed.x = atof(pElem->Attribute("x"));
	if (pElem->Attribute("y"))
		ed.y = atof(pElem->Attribute("y"));
	UpdateExtents(ed);
	m_elemList.push_back(ed);
}

void SvgFile::LoadLine(TinyXML2::XMLElement* pElem)
{
	// Load SVG <line> element.
	//

	ElemDesc ed;
	ed.type = ElemType::Line;
	ed.id = pElem->Attribute("id");
	ed.style = pElem->Attribute("style");

	// must have height/width, x/y optional
	ed.x = atof(pElem->Attribute("x1"));
	ed.y = atof(pElem->Attribute("y1"));
	ed.w = atof(pElem->Attribute("x2"));
	ed.h = atof(pElem->Attribute("y2"));
	UpdateExtents(ed);
	m_elemList.push_back(ed);
}

void SvgFile::LoadPath(TinyXML2::XMLElement* pElem, MAT3* pmat)
{
	// Load path node.
	//
	// Inputs:
	//		pElem = 
	//		transform to apply if non-null
	//
	// path commands:
	//
	//		M = moveto
	//		L = lineto
	//		H = horizontal lineto
	//		V = vertical lineto
	//		C = curveto, cubic bezier curve
	//				C x1 y1, x2 y2, x y		=> x1,y1 = start control point
	//										=> x2,y2 = end control point
	//										=> x,y = end point
	//				c dx1 dy1, dx2 dy2, dx dy
	//		S = smooth curveto				=> for connecting C curves smoothly
	//				S x2 y2, x y 
	//				s dx2 dy2, dx dy
	//		Q = quadratic Bézier curve
	//				 Q x1 y1, x y			=> x1,y1 = control point
	//										=> x,y = end point (curve is drawn from "current point" to here)
	//				 q dx1 dy1, dx dy
	//		T = smooth quadratic Bézier curveto		=> for connecting multiple Q curves
	//		A = elliptical Arc
	//			A rx ry x-axis-rotation large-arc-flag sweep-flag x y
	//			a rx ry x-axis-rotation large-arc-flag sweep-flag dx dy
	//		Z = closepath
	//

	ElemDesc ed;
	ed.type = ElemType::Path;
	ed.id = pElem->Attribute("id");
	ed.style = pElem->Attribute("style");

	// from docs:
	//	If a relative moveto (m) appears as the first element of the path, then it is treated as a pair of absolute coordinates. 
	//	In this case, subsequent pairs of coordinates are treated as relative even though the initial moveto is interpreted as an
	//	absolute moveto. 
	//
	// can look like this:
	//		"m -2736.4956,-20152.497 9,675 0,117 2,1215 -320,7 -15,970 -1,863 -5,823 1,238 -971,-4 -127,0 40,35 72,50 61,38 41,31 71,12 z"
	// or this:
	//		"M72.9333 1.56458 C82.1333 3.56458, 285.333 63.2979, 295.6 67.1646 C303.467 69.9646, 312.667 80.3646, 315.2 89.1646 ...
	//

	XString path = pElem->Attribute("d");

	// ensure commands detached
	// (ie "M3,5", "M 3,5", "M3 5")
	for (int i = 0; i < path.GetLength(); ++i)
	{
		if (isalpha(path[i]))
			path.Insert(i + 1, ' ');
	}

	PathCmd cmd = PathCmd::None;

	// split all tokens
	int tokenCount = path.Tokenize(", ");
	for (int i = 0; i < tokenCount; ++i)
	{
		PathEntry pe;
		XString s = path.GetToken(i);

		if (isalpha(s[0]))
		{
			cmd = CharToPathCmd(s[0]);

			// "close path" may be legal mid-path, but for now i'm only
			// supporting at the end (entire path either closed or not)
			if (cmd == PathCmd::Z || cmd == PathCmd::z)
				ed.IsPathClosed = true;
		}
		else
		{
			pe.x = path.GetToken(i).GetDouble();
			pe.y = path.GetToken(i + 1).GetDouble();
			++i;

			pe.cmd = cmd;
			ed.path.push_back(pe);
		}
	}

	if (pmat)
	{
		// if translating active, apply it, but only to 
		// first absolute position
		//ed.path[0].x += delta->x;
		//ed.path[0].y += delta->y;
	}

	UpdateExtents(ed);
	m_elemList.push_back(ed);
}

void SvgFile::LoadPolygon(TinyXML2::XMLElement* pElem)
{
	// Load svg <polygon> node.
	//

	ElemDesc ed;
	ed.type = ElemType::Polygon;
	ed.id = pElem->Attribute("id");
	ed.style = pElem->Attribute("style");

	// <polygon points="200,10 250,190 160,210" style="fill:lime;stroke:purple;stroke-width:1" />

	XString pts = pElem->Attribute("points");
	int tokenCount = pts.Tokenize(" ");
	for (int i = 0; i < tokenCount; ++i)
	{
		PathEntry pe;
		XString s = pts.GetToken(i);

		// coordinate pair
		int pairTokens = s.Tokenize(",");
		if (pairTokens == 2)
		{
			pe.x = s.GetToken(0).GetDouble();
			pe.y = s.GetToken(1).GetDouble();
		}
		else
		{
			Logger::Write(__FUNCTION__, "Invalid token count: %d", pairTokens);
			assert(false);
		}
		ed.path.push_back(pe);
	}

	UpdateExtents(ed);
	m_elemList.push_back(ed);
}

void SvgFile::LoadPolyLine(TinyXML2::XMLElement* pElem)
{
	// Load svg <polyline> node.
	//

	assert(IsPolyLine(pElem));

	ElemDesc ed;
	ed.type = ElemType::PolyLine;
	ed.id = pElem->Attribute("id");
	ed.style = pElem->Attribute("style");

	// <polyline points="200,10 250,190 160,210" style="fill:lime;stroke:purple;stroke-width:1" />

	XString pts = pElem->Attribute("points");
	int tokenCount = pts.Tokenize(" ");
	for (int i = 0; i < tokenCount; ++i)
	{
		PathEntry pe;
		XString s = pts.GetToken(i);

		// coordinate pair
		int pairTokens = s.Tokenize(",");
		if (pairTokens == 2)
		{
			pe.x = s.GetToken(0).GetDouble();
			pe.y = s.GetToken(1).GetDouble();
		}
		else
		{
			Logger::Write(__FUNCTION__, "Invalid token count: %d", pairTokens);
			assert(false);
		}
		ed.path.push_back(pe);
	}

	UpdateExtents(ed);
	m_elemList.push_back(ed);
}

bool SvgFile::Save(const char* fileName)
{
	// Save SVG file.
	//

	XString strFileName;
	if (fileName == nullptr)
		strFileName = ms_FileName;
	else
		strFileName = fileName;

	TinyXML2::XMLDocument doc;

	TinyXML2::XMLElement* pRoot = doc.NewElement("svg");

	// bounding box
	RectD ext = GetExtents();
	double minx = floor(ext.x0);
	double maxx = ceil(ext.x1);
	double miny = floor(ext.y0);
	double maxy = ceil(ext.y1);
	XString strBox = XString::Format("%d %d %d %d", (int)minx, (int)miny, (int)maxx, (int)maxy);
	pRoot->SetAttribute("viewbox", strBox.c_str());

	//	<svg width = "733.778mm" height = "733.778mm"
	//	xmlns = "http://www.w3.org/2000/svg" xmlns:xlink = "http://www.w3.org/1999/xlink"  version = "1.2" baseProfile = "tiny">
	//	<title>SVG Contour Export</title>
	//	<desc>Generated with Qt</desc>
	//	<defs>
	//	</defs>
	//	<g fill = "none" stroke = "black" stroke-width = "1" fill-rule = "evenodd" stroke-linecap = "square" stroke-linejoin = "bevel" >

	doc.InsertFirstChild(pRoot);

	for (auto el : m_elemList)
	{
		XString id = el.id;

		switch (el.type)
		{
		case ElemType::Path:
			{
				TinyXML2::XMLElement* pElement = doc.NewElement("path");
				if (id.IsEmpty() == false)
					pElement->SetAttribute("id", el.id.c_str());
				XString points = "m";
				for (int i = 0; i < el.path.size(); ++i)
				{
					XString strx = MarkLib::TrimFloat(el.path[i].x, 1);
					XString stry = MarkLib::TrimFloat(el.path[i].y, 1);
					points += XString::Format(" %s,%s", strx.c_str(), stry.c_str());
				}
				if (el.IsPathClosed)
					points += " z";
				pElement->SetAttribute("d", points.c_str());
				pRoot->InsertEndChild(pElement);
			}
			break;
		case ElemType::PolyLine:
			{
				TinyXML2::XMLElement* pElement = doc.NewElement("polyline");
				if (id.IsEmpty() == false)
					pElement->SetAttribute("id", el.id.c_str());
				XString points;
				for (int i = 0; i < el.path.size(); ++i)
				{
					XString strx = MarkLib::TrimFloat(el.path[i].x, 1);
					XString stry = MarkLib::TrimFloat(el.path[i].y, 1);
					points += XString::Format(" %s,%s", strx.c_str(), stry.c_str());
				}
				pElement->SetAttribute("points", points.c_str());
				pRoot->InsertEndChild(pElement);
			}
			break;
		case ElemType::Rectangle:
			break;
		case ElemType::Line:
			break;
		default:
			Logger::Write(__FUNCTION__, "Unhandled element type: %d", (int)el.type);
			assert(false);
			break;
		}
	}

	// create backup
	XString strBackFile = strFileName + ".bak";
	if (MarkLib::FileExists(strFileName.c_str()))
	{
		if (MarkLib::FileCopy(strFileName.c_str(), strBackFile.c_str(), true) == false)
		{
			Logger::Write(__FUNCTION__, "Error creating backup file: %s", strBackFile.c_str());
		}
	}

	// write file
	TinyXML2::XMLError status = doc.SaveFile(strFileName.c_str());
	if (status != TinyXML2::XMLError::XML_SUCCESS)
	{
		Logger::Write(__FUNCTION__, "Error saving XML file: %d", static_cast<int>(status));
		return false;
	}

	mb_IsDirty = false;

	return true;
}

bool SvgFile::Export(const char* fileName, bool visible)
{
	// Export contents of SVG file.
	// 
	// Inputs:
	//		fileName = full path/name of output file
	//		visible  = true => output visible items only, else all
	//
	//TODO:
	//	can imagine all sorts of options, including output format, but
	//  my immediate need is to simply write visible elements to an svg file
	//

	assert(fileName);
	assert(visible == true);		// other opts not yet implemented

	TinyXML2::XMLDocument doc;

	TinyXML2::XMLElement* pRoot = doc.NewElement("svg");
	doc.InsertFirstChild(pRoot);

	for (auto el : m_elemList)
	{
		if (visible && el.IsVisible)
		{
			XString id = el.id;

			switch (el.type)
			{
			case ElemType::Path:
				{
					TinyXML2::XMLElement* pElement = doc.NewElement("path");
					if (id.IsEmpty() == false)
						pElement->SetAttribute("id", el.id.c_str());
					XString points = "m";
					for (int i = 0; i < el.path.size(); ++i)
					{
						XString strx = MarkLib::TrimFloat(el.path[i].x, 1);
						XString stry = MarkLib::TrimFloat(el.path[i].y, 1);
						points += XString::Format(" %s,%s", strx.c_str(), stry.c_str());
					}
					if (el.IsPathClosed)
						points += " z";
					pElement->SetAttribute("d", points.c_str());
					pRoot->InsertEndChild(pElement);
				}
				break;
			case ElemType::Rectangle:
				break;
			case ElemType::Line:
				break;
			default:
				Logger::Write(__FUNCTION__, "Unhandled element type: %d", (int)el.type);
				assert(false);
				break;
			}
		}
	}

	// write file
	TinyXML2::XMLError status = doc.SaveFile(fileName);
	if (status != TinyXML2::XMLError::XML_SUCCESS)
	{
		Logger::Write(__FUNCTION__, "Error saving XML file: %d", static_cast<int>(status));
		return false;
	}

	return true;
}

XString SvgFile::GetFileName()
{
	return ms_FileName;
}

bool SvgFile::IsDirty()
{
	return mb_IsDirty;
}

RectD SvgFile::GetExtents()
{
	// Return extents for entire svg file.
	//

	RectD ext;

	bool init = false;
	for (auto el : m_elemList)
	{
		if (init == false)
		{
			ext = el.ext;
			init = true;
		}
		else
		{
			if (ext.x0 > el.ext.x0)
				ext.x0 = el.ext.x0;
			if (ext.x1 < el.ext.x1)
				ext.x1 = el.ext.x1;
			if (ext.y0 > el.ext.y0)
				ext.y0 = el.ext.y0;
			if (ext.y1 < el.ext.y1)
				ext.y1 = el.ext.y1;
		}
	}

	return ext;
}

void SvgFile::SetMousePos(double x, double y)
{
	// Set current mouse cursor position,
	// world units.
	//

	m_ptMouse.x = x;
	m_ptMouse.y = y;
}

void SvgFile::DeselectAll(bool deep)
{
	for (int i = 0; i < m_elemList.size(); ++i)
	{
		m_elemList[i].IsSelected = false;
		if (deep)
		{
			for (int j = 0; j < m_elemList[i].path.size(); ++j)
				m_elemList[i].path[j].IsSelected = false;
		}
	}
}

void SvgFile::SelectAtPoint(double x, double y, bool deselect)
{
	// Select item at point (x,y), if any.
	//

	// first, see if we are over an active select point
	//if (GetSelectPoint(x, y))
		//return;
	
	if (deselect)
		DeselectAll();

	PointD pt = PointD(x, y);
						
	for (int i = 0; i < m_elemList.size(); ++i)
	{
		// only visible items can be selected
		if (m_elemList[i].IsVisible)
		{
			RectD ext = m_elemList[i].ext;
			ext.Normalize();
			ext.Grow(PIXELS_CLOSE);
			if (ext.Contains(x, y))
			{
				switch (m_elemList[i].type)
				{
				case ElemType::Path:
					{
						PointD ptLast = PointD(m_elemList[i].path[0].x, m_elemList[i].path[0].y);
						for (int pi = 1; pi < m_elemList[i].path.size(); ++pi)
						{
							RectD rect;
							rect.x0 = ptLast.x;
							rect.y0 = ptLast.y;
							rect.x1 = rect.x0 + m_elemList[i].path[pi].x;
							rect.y1 = rect.y0 + m_elemList[i].path[pi].y;
							ptLast = PointD(rect.x1, rect.y1);

							if (CloseToLine(pt, rect))
							{
								m_elemList[i].IsSelected = true;
								break;
							}
						}
					}
					break;
				case ElemType::PolyLine:
					{
						for (int pi = 1; pi < m_elemList[i].path.size(); ++pi)
						{
							RectD rect;
							rect.x0 = m_elemList[i].path[pi-1].x;
							rect.y0 = m_elemList[i].path[pi-1].y;
							rect.x1 = m_elemList[i].path[pi].x;
							rect.y1 = m_elemList[i].path[pi].y;

							if (CloseToLine(pt, rect))
							{
								m_elemList[i].IsSelected = true;
								break;
							}
						}
					}
					break;
				case ElemType::Line:
						{
							RectD rect;
							rect.x0 = m_elemList[i].x; 
							rect.y0 = m_elemList[i].y;
							rect.x1 = m_elemList[i].x + m_elemList[i].w; 
							rect.y1 = m_elemList[i].y + m_elemList[i].h;

							if (CloseToLine(pt, rect))
								m_elemList[i].IsSelected = true;
						}
						break;
				case ElemType::Rectangle:
						{
							RectD rect;
							rect.x0 = m_elemList[i].x; 
							rect.y0 = m_elemList[i].y;
							rect.x1 = m_elemList[i].x + m_elemList[i].w; 
							rect.y1 = m_elemList[i].y + m_elemList[i].h;

							// has to be on line, not within rectangle
							if ((fabs(rect.x0 - x) < 1.0) || (fabs(rect.x1 - x) < 1.0))
							{
								if (y > rect.y0 && y < rect.y1)
									m_elemList[i].IsSelected = true;
							}
							if ((fabs(rect.y0 - y) < 1.0) || (fabs(rect.y1 - y) < 1.0))
							{
								if (x > rect.x0 && x < rect.x1)
									m_elemList[i].IsSelected = true;
							}
						}
						break;
				default:
					Logger::Write(__FUNCTION__, "Unhandled element type: %d", (int)m_elemList[i].type);
					assert(false);
					break;
				}
			}
		}
	}
}

void SvgFile::SelectInRect(double x0, double y0, double x1, double y1, bool deselect)
{
	// Select all elements within rectangle.
	//

	if (deselect)
		DeselectAll();

	RectD rect(x0, y0, x1, y1);

	for (int i = 0; i < m_elemList.size(); ++i)
	{
		// only visible elements can be selected
		if (m_elemList[i].IsVisible)
		{
			// if the 2 rects intersect, it's possible the shapes
			// intersect; still need to drill-down by specific shape
			RectD ext = m_elemList[i].ext;
			if (IntersectRectRect(rect, ext))
			{
				switch (m_elemList[i].type)
				{
				case ElemType::Path:
				case ElemType::Polygon:
				case ElemType::PolyLine:
					m_elemList[i].IsSelected = true;
					break;
				case ElemType::Line:
					m_elemList[i].IsSelected = true;
					break;
				case ElemType::Rectangle:
					m_elemList[i].IsSelected = true;
					break;
				default:
					Logger::Write(__FUNCTION__, "Unhandled element type: %d", (int)m_elemList[i].type);
					assert(false);
					break;
				}
			}
		}
	}
}

int SvgFile::GetSelectedElementCount()
{
	// Return # of elements selected.
	//

	int count = 0;

	for (int i = 0; i < m_elemList.size(); ++i)
	{
		if (m_elemList[i].IsSelected)
			++count;
	}

	return count;
}

int SvgFile::GetSelectedElementIndex()
{
	// Return index of first selected element.
	//

	for (int i = 0; i < m_elemList.size(); ++i)
	{
		if (m_elemList[i].IsSelected)
			return i;
	}

	return -1;
}

int SvgFile::GetVisibleElementCount()
{
	// Return # of elements that are visible.
	//

	int count = 0;

	for (int i = 0; i < m_elemList.size(); ++i)
	{
		if (m_elemList[i].IsVisible)
			++count;
	}

	return count;
}

void SvgFile::SetElementVisible(int index, bool render)
{
	if (index >= 0 && index < GetElementCount())
	{
		m_elemList[index].IsVisible = render;
	}
}

bool SvgFile::IsElementVisible(int index)
{
	if (index >= 0 && index < GetElementCount())
		return m_elemList[index].IsVisible;

	return false;
}

void SvgFile::SelectPathSegment(int elemIndex, int segIndex)
{
	// Select a segment in a path element.
	//
	// Inputs:
	//		elemIndex = svg element index (assumed to be path)
	//		segIndex  = segment index within path
	//

	if (elemIndex >= 0 && elemIndex < GetElementCount())
	{
		assert(m_elemList[elemIndex].type == ElemType::Path);

		// no other elements may be selected
		DeselectAll(false);

		if (segIndex >= 0 && segIndex < m_elemList[elemIndex].path.size())
		{
			m_elemList[elemIndex].path[segIndex].IsSelected = !m_elemList[elemIndex].path[segIndex].IsSelected;
			m_elemList[elemIndex].IsSelected = true;
		}
	}
}

bool SvgFile::IsPathSegmentSelected(int elemIndex, int segIndex)
{
	bool selected = false;

	if (elemIndex >= 0 && elemIndex < GetElementCount())
	{
		assert(m_elemList[elemIndex].type == ElemType::Path);
		if (segIndex >= 0 && segIndex < m_elemList[elemIndex].path.size())
			selected = m_elemList[elemIndex].path[segIndex].IsSelected;
	}

	return selected;
}

bool SvgFile::FindPathSegment(PointD pt0, PointD pt1, double epsilon, int& elemIndex, int& segIndex, int skipElem)
{
	// Given 2 points, see if they define a path segment.
	//
	// Inputs:
	//		pt0/pt1 = segment endpoints
	//		epsilon = delta that defines how close 2 points must be to be equal
	//		skipElem = element to skip in search, -1 to search all
	//
	// Outputs:
	//		elemIndex = index of element containing path segment
	//		segIndex = index of matching segment within elemIndex
	//		return = true if match was found
	//
	
	bool found = false;

	PointD curPos(0.0, 0.0);
	for (int pi = 0; pi < GetElementCount(); ++pi)
	{
		if (skipElem == pi)
			continue;

		ElemDesc* pElem = &m_elemList[pi];
		if (pElem->type == ElemType::Path)
		{
			for (int si = 0; si < pElem->path.size() - 1; ++si)
			{
				curPos.x += pElem->path[si].x;
				curPos.y += pElem->path[si].y;
				PointD nextPos;
				nextPos.x = curPos.x + pElem->path[si + 1].x;
				nextPos.y = curPos.y + pElem->path[si + 1].y;

				// points could be at either end of line segment
				if ((abs(curPos.x - pt0.x) < epsilon && abs(curPos.y - pt0.y) < epsilon) && (abs(nextPos.x - pt1.x) < epsilon && abs(nextPos.y - pt1.y) < epsilon)
				||  (abs(curPos.x - pt1.x) < epsilon && abs(curPos.y - pt1.y) < epsilon) && (abs(nextPos.x - pt0.x) < epsilon && abs(nextPos.y - pt0.y) < epsilon))
				{
					printf("found one!");
					elemIndex = pi;
					segIndex = si;
					found = true;
				}
			}
		}
	}

	return found;
}

bool SvgFile::CloseToLine(PointD pt, RectD line)
{
	// Helper function to test if point is "close to"
	// line segment. This is specifically for user clicking 
	// near a line, so distance in pixels is relevant.
	//

	bool isClose = false;

	// verify within bounding rect (+ delta) so we are 
	// only testing line segment, not extended line
	RectD rect = line;
	rect.Grow(PIXELS_CLOSE * mf_PixelSize);
	if (rect.Contains(pt.x, pt.y))
	{
		double distance = DistancePointToLine2D(pt, line);

		if (distance < mf_PixelSize * PIXELS_CLOSE)
			isClose = true;
	}

	return isClose;
}

void SvgFile::AddPolyLine(std::vector<PointD>& ptList)
{
	// Add a polyline element to svg file.
	//

	if (ptList.size() == 0)
		return;

	ElemDesc elem;
	elem.type = ElemType::PolyLine;
	elem.id = XString::Format("polyline%d", GetElementCount() + 1);

	PathEntry pe;
	for (int i = 0; i < ptList.size(); ++i)
	{
		pe.x = ptList[i].x;
		pe.y = ptList[i].y;
		elem.path.push_back(pe);
	}

	UpdateExtents(elem);
	m_elemList.push_back(elem);

	mb_IsDirty = true;
}

SvgFile::PathCmd SvgFile::CharToPathCmd(char c)
{
	switch (c)
	{
	case 'M': return PathCmd::M;	// upper case = absolute
	case 'm': return PathCmd::m;	// lower case = relative
	case 'L': return PathCmd::L;
	case 'l': return PathCmd::l;
	case 'H': return PathCmd::H;
	case 'h': return PathCmd::h;
	case 'V': return PathCmd::V;
	case 'v': return PathCmd::v;
	case 'C': return PathCmd::C;
	case 'c': return PathCmd::c;
	case 'S': return PathCmd::S;
	case 's': return PathCmd::s;
	case 'Q': return PathCmd::Q;
	case 'q': return PathCmd::q;
	case 'T': return PathCmd::T;
	case 't': return PathCmd::t;
	case 'A': return PathCmd::A;
	case 'a': return PathCmd::a;
	case 'Z': return PathCmd::Z;
	case 'z': return PathCmd::z;
	};

	return PathCmd::None;
}

void SvgFile::UpdateExtents(ElemDesc& el)
{
	// Update element extents.
	//

	switch (el.type)
	{
	case ElemType::Rectangle:
		el.ext.x0 = el.x;
		el.ext.x1 = el.x + el.w;
		el.ext.y0 = el.y;
		el.ext.y1 = el.y + el.h;
		break;
	case ElemType::Polygon:
	case ElemType::PolyLine:
		{
			bool init = false;
			PointD curPoint;
			for (int i = 0; i < el.path.size(); ++i)
			{
				curPoint.x = el.path[i].x;
				curPoint.y = el.path[i].y;

				if (init == false)
				{
					el.ext.x0 = el.ext.x1 = curPoint.x;
					el.ext.y0 = el.ext.y1 = curPoint.y;

					init = true;
				}
				else
				{
					// update extents
					if (el.ext.x0 > curPoint.x)
						el.ext.x0 = curPoint.x;
					if (el.ext.x1 < curPoint.x)
						el.ext.x1 = curPoint.x;
					if (el.ext.y0 > curPoint.y)
						el.ext.y0 = curPoint.y;
					if (el.ext.y1 < curPoint.y)
						el.ext.y1 = curPoint.y;
				}
			}
		}
		break;
	case ElemType::Path:
		{
			bool init = false;
			PointD curPoint;
			for (int i = 0; i < el.path.size(); ++i)
			{
				if ((el.path[i].cmd == PathCmd::M) || ((i == 0) && (el.path[i].cmd == PathCmd::m)))
				{
					// absolute move to
					if (init == false)
					{
						el.ext.x0 = el.ext.x1 = el.path[i].x;
						el.ext.y0 = el.ext.y1 = el.path[i].y;

						init = true;
					}

					// an absolute move - intialized or not - makes our current
					// point that point
					curPoint.x = el.path[i].x;
					curPoint.y = el.path[i].y;
				}
				else
				{
					// shouldn't be able to get into relative moves
					// (or any unsupported command) before init
					assert(init);

					// relative move to
					curPoint.x += el.path[i].x;
					curPoint.y += el.path[i].y;
				}

				// update extents
				if (el.ext.x0 > curPoint.x)
					el.ext.x0 = curPoint.x;
				if (el.ext.x1 < curPoint.x)
					el.ext.x1 = curPoint.x;
				if (el.ext.y0 > curPoint.y)
					el.ext.y0 = curPoint.y;
				if (el.ext.y1 < curPoint.y)
					el.ext.y1 = curPoint.y;
			}
		}
		break;
	default:
		Logger::Write(__FUNCTION__, "Unhandled element type: %d", (int)el.type);
		assert(false);
	}
}

PointD SvgFile::TransformToDelta(XString strTransform)
{
	// Convert a transform string to an xy delta.
	//
	// Only translations are currently supported:
	//		transform="translate(51.504435,51.502611)
	//

	PointD d;

	strTransform.Trim();
	assert(strTransform.BeginsWithNoCase("translate"));
	int tokenCount = strTransform.Tokenize("(),");
	if (tokenCount >= 3)
	{
		d.x = strTransform.GetToken(1).GetDouble();
		d.y = strTransform.GetToken(2).GetDouble();
	}

	return d;
}

MAT3 SvgFile::TransformToMatrix(XString strTransformEntry)
{
	// Convert a transform string to a 3x3 matrix.
	//
	// There can be multiple transforms in a single element.
	// For example:
	//		transform="translate(50,0) rotate(30)"
	//      or
	//		transform="translate(100, 0) scale(-1, 1)"
	//

	MAT3 mat;

	strTransformEntry.Trim();

	// build a list of transform functions
	std::vector<XString> transformList;
	int pos = strTransformEntry.Find(')');
	while (pos > -1)
	{
		XString str = strTransformEntry.Left(pos + 1);
		transformList.push_back(str);
		strTransformEntry.DeleteLeft(pos + 1);
		pos = strTransformEntry.Find(')');
	}

	for each (XString strTransform in transformList)
	{
		if (strTransform.BeginsWithNoCase("matrix"))
		{
			// see: http://tutorials.jenkov.com/svg/svg-transformation.html

			MAT3 m;

			m(2, 0) = 0.0; m(2, 1) = 0.0; m(2, 2) = 1.0;

			// get parameters
			double a, b, c, d, e, f;
			strTransform.DeleteLeft(6);
			int tokenCount = strTransform.Tokenize("(),");
			if (tokenCount == 6)
			{
				a = strTransform.GetToken(0).GetDouble();
				b = strTransform.GetToken(1).GetDouble();
				c = strTransform.GetToken(2).GetDouble();
				d = strTransform.GetToken(3).GetDouble();
				e = strTransform.GetToken(4).GetDouble();
				f = strTransform.GetToken(5).GetDouble();

				m(0, 0) = a;
				m(0, 1) = c;
				m(0, 2) = e;

				m(1, 0) = b;
				m(1, 1) = d;
				m(1, 2) = f;

				// concatenate the matrices
				mat *= m;
			}
			else
			{
				Logger::Write(__FUNCTION__, "Invalid matrix() entry: %s", strTransform.c_str());
			}

			// it's a 3x3 matrix, but only the top 2 rows can be specified:
			//
			//		a  c  e		=> m[0][0]  m[1][0]  m[2][0]
			//		b  d  f        m[0][1]  m[1][1]  m[2][1]
			//		0  0  1        m[0][2]  m[1][2]  m[2][2]
			//
			//	Translate
			//
			//		1  0  tx
			//		0  1  ty
			//		0  0   1
			//
			//		matrix(1, 0, 0, 1, tx, ty)
			//
			//	Rotate
			//
			//		cos(a) -sin(a)  0
			//		sin(a)  cos(a)  0
			//		0        0   1
			//		Note: the values for cos(a) and sin(a) have to be precomputed before being inserted into the matrix.
			//	
			//		matrix(cos(a), sin(a), -sin(a), cos(a), 0, 0)
			//
			//	Scale
			//
			//		sx  0   0
			//		0   sy  0
			//		0   0   1
			//
			//		matrix(sx, 0, 0, sy, 0, 0)
			//
			//  SkewX
			//
			//		1  tan(a)  0
			//		0       1  0
			//		0       0  1
			//
			//		matrix(1, 0, tan(a), 1, 0, 0)
			//
			//		The tan(a) value has to be precomputed before being inserted into the matrix() function.
			//
			//
			//  SkewY
			//
			//		1       0  0
			//		tan(a)  1  0
			//		0       0  1
			//
			//		matrix(1, tan(a), 0, 1, 0, 0)
			//

			//m[0][0] = a;  m[1][0] = c;  m[2][0] = e;
			//m[0][1] = b;  m[1][1] = d;  m[2][1] = f;
			//m[0][2] = 0.0;  m[1][2] = 0.0; m[2][2] = 1.0;
		}
		else if (strTransform.BeginsWithNoCase("translate"))
		{
			strTransform.DeleteLeft(9);

			int tokenCount = strTransform.Tokenize("(),");
			if (tokenCount == 2)
			{
				//	1  0  tx
				//	0  1  ty
				//	0  0   1

				MAT3 m;

				m(0, 2) = strTransform.GetToken(0).GetDouble();
				m(1, 2) = strTransform.GetToken(1).GetDouble();

				// concatenate the matrices
				mat *= m;
			}
			else
			{
				Logger::Write(__FUNCTION__, "Invalid translate() entry: %s", strTransform.c_str());
			}
		}
		else if (strTransform.BeginsWithNoCase("scale"))
		{
			strTransform.DeleteLeft(5);

			int tokenCount = strTransform.Tokenize("(),");
			if (tokenCount == 2)
			{
				//  sx  0   0
				//  0   sy  0
				//	0   0   1

				MAT3 m;

				m(0, 0) = strTransform.GetToken(0).GetDouble();
				m(1, 1) = strTransform.GetToken(1).GetDouble();

				// concatenate the matrices
				mat *= m;
			}
			else
			{
				Logger::Write(__FUNCTION__, "Invalid scale() entry: %s", strTransform.c_str());
			}
		}
		else if (strTransform.BeginsWithNoCase("rotate"))
		{
			//<!--rotation is done around the point 0, 0 -->
			//	<rect x = "0" y = "0" width = "10" height = "10" fill = "red"
			//	transform = "rotate(100)" / >

			//	<!--rotation is done around the point 10, 10 -->
			//	<rect x = "0" y = "0" width = "10" height = "10" fill = "green"
			//	transform = "rotate(100,10,10)" / >

			strTransform.DeleteLeft(5);

			int tokenCount = strTransform.Tokenize("(),");
			if (tokenCount == 1)
			{
				//	cos(a) -sin(a)  0
				//	sin(a)  cos(a)  0
				//	0        0		1

				MAT3 m;

				double a = strTransform.GetToken(0).GetDouble();

				m(0, 0) = cos(a);
				m(0, 1) = -sin(a);

				m(1, 0) = sin(a);
				m(1, 1) = cos(a);

				// concatenate the matrices
				mat *= m;
			}
			else if (tokenCount == 3)
			{
				//TODO:
				// rotate about 0,0 then translate
			}
			else
			{
				Logger::Write(__FUNCTION__, "Invalid rotate() entry: %s", strTransform.c_str());
			}
		}
		else if (strTransform.BeginsWithNoCase("skewx"))
		{
			//TODO:
		}
		else if (strTransform.BeginsWithNoCase("skewy"))
		{
			//TODO:
		}
	}

	return mat;
}

bool SvgFile::IsGroup(TinyXML2::XMLElement* pElem)
{
	// Return true if pElem is a "group" element.
	//

	XString strGroup = "g";

	return strGroup.CompareNoCase(pElem->Value());
}

bool SvgFile::IsPath(TinyXML2::XMLElement* pElem)
{
	// Return true if pElem is a "path" element.
	//

	XString strPath = "path";

	return strPath.CompareNoCase(pElem->Value());
}

bool SvgFile::IsRect(TinyXML2::XMLElement* pElem)
{
	// Return true if pElem is a "rect" element.
	//

	XString strRect = "rect";

	return strRect.CompareNoCase(pElem->Value());
}

bool SvgFile::IsPoly(TinyXML2::XMLElement* pElem)
{
	// Return true if pElem is a "polygon" element.
	//

	XString strPolygon = "polygon";

	return strPolygon.CompareNoCase(pElem->Value());
}

bool SvgFile::IsPolyLine(TinyXML2::XMLElement* pElem)
{
	// Return true if pElem is a "polyline" element.
	//

	XString strPolyLine = "polyline";

	return strPolyLine.CompareNoCase(pElem->Value());
}

bool SvgFile::IsLine(TinyXML2::XMLElement* pElem)
{
	// Return true if pElem is a "line" element.
	//

	XString strLine = "line";

	return strLine.CompareNoCase(pElem->Value());
}

bool SvgFile::IsMeta(TinyXML2::XMLElement* pElem)
{
	// Return true if pElem is a "metadata" element.
	//

	XString strMeta = "metadata";

	return strMeta.CompareNoCase(pElem->Value());
}

bool SvgFile::IsDefs(TinyXML2::XMLElement* pElem)
{
	// Return true if pElem is a "defs" element.
	//

	XString strDefs = "defs";

	return strDefs.CompareNoCase(pElem->Value());
}

bool SvgFile::IsClipPath(TinyXML2::XMLElement* pElem)
{
	// Return true if pElem is a "clipPath" element.
	//

	XString strClipPath = "clipPath";

	return strClipPath.CompareNoCase(pElem->Value());
}

bool SvgFile::IsTitle(TinyXML2::XMLElement* pElem)
{
	// Return true if pElem is a "title" element.
	//

	XString strTitle = "title";

	return strTitle.CompareNoCase(pElem->Value());
}

bool SvgFile::IsDesc(TinyXML2::XMLElement* pElem)
{
	// Return true if pElem is a "desc" element.
	//

	XString strDesc = "desc";

	return strDesc.CompareNoCase(pElem->Value());
}

void SvgFile::SetPixelSize(double pixelSize)
{
	mf_PixelSize = pixelSize;
}

void SvgFile::Delete()
{
	// Delete selected elements.
	// 

	for (int i = 0; i < static_cast<int>(m_elemList.size()); ++i)
	{
		if (m_elemList[i].IsSelected)
		{
			m_elemList.erase(m_elemList.cbegin() + i);
			--i;
		}
	}

	mb_IsDirty = true;
}

void SvgFile::DeleteSubRange(int elemIndex, int segStart, int segEnd, XString name1, XString name2)
{
	// Delete a subrange of segments from a path.
	// 
	// Inputs:
	//		elemIndex = index of target path element
	//		segStart = first segment to delete within target path
	//		segEnd = last segment to delete within target path
	//		name1 = new path name #1, if old path bisected
	//		name2 = new path name #2, if old path bisected
	//
	// There are 3 scenarios:
	//
	//		1. The range is in the middle somewhere, so the cut will
	//		   divide the path into 2. Each of those new paths will
	//		   need a new name.
	//		2. The range is at the beginning, so the anchor point will
	//		   have to move.
	//		3. Cut is at the end. No new path or change to anchor
	//		   needed.
	//

	ElemDesc* pDesc = &m_elemList[elemIndex];
	assert(pDesc->type == ElemType::Path);

	std::vector<PathEntry> path = pDesc->path;		// source path
	std::vector<PointD> pathAbs;					// matching absolute positions

	PointD curPos(0.0, 0.0);
	for (int i = 0; i < pDesc->path.size(); ++i)
	{
		curPos.x += pDesc->path[i].x;
		curPos.y += pDesc->path[i].y;

		pathAbs.push_back(curPos);
	}

	bool wasClosed = pDesc->IsPathClosed;
	
	if (segStart == 0)
	{
		// deleting the anchor point

		// erase() does not include last element
		pDesc->path.erase(pDesc->path.begin(), pDesc->path.begin() + segEnd + 1);
		pDesc->path[0].cmd = PathCmd::M;
		
		// set new anchor
		pDesc->path[0].x = pathAbs[segEnd + 1].x;
		pDesc->path[0].y = pathAbs[segEnd + 1].y;

		// path no longer closed
		pDesc->IsPathClosed = false;

		if (wasClosed)
		{
			// since path no longer closed, explicitly add 
			// last segment
			PathEntry pe;
			pe.cmd = PathCmd::m;
			pe.x = pathAbs[0].x-pathAbs.back().x;
			pe.y = pathAbs[0].y-pathAbs.back().y;
			pDesc->path.push_back(pe);
		}

	}
	else if (segEnd == static_cast<int>(pDesc->path.size()) - 1)
	{
		// trimming end of path

		pDesc->path.erase(pDesc->path.begin() + segStart, pDesc->path.begin() + segEnd + 1);

		// path no longer closed
		pDesc->IsPathClosed = false;
	}
	else
	{
		// bisecting the path

		// keep first path
		pDesc->path.erase(pDesc->path.begin() + segStart + 1, pDesc->path.end());
		pDesc->id = name1;
		pDesc->IsPathClosed = false;
		
		// create second path
		ElemDesc ed;
		ed.type = ElemType::Path;
		ed.id = name2;
		ed.style = pDesc->style;
		ed.IsPathClosed = false;

		PathEntry pe;
		pe.cmd = PathCmd::m;
		pe.x = pathAbs[segEnd + 1].x;
		pe.y = pathAbs[segEnd + 1].y;
		ed.path.push_back(pe);
		for (int i = segEnd + 2; i < path.size(); ++i)
		{
			pe.x = path[i].x;
			pe.y = path[i].y;
			ed.path.push_back(pe);
		}

		if (wasClosed)
		{
			// since path no longer closed, explicitly add 
			// last segment
			path[0].x = pathAbs[0].x-pathAbs.back().x;
			path[0].y = pathAbs[0].y-pathAbs.back().y;
			ed.path.push_back(path[0]);
		}

		// insert after first path
		m_elemList.insert(m_elemList.cbegin() + elemIndex + 1, ed);
	}

	mb_IsDirty = true;
}

int SvgFile::MergePaths(double epsilon)
{
	// Remove overlapping path segments.
	//
	// Not exactly a merge operation, really delete 
	// duplicates, but can see option to merge
	// any remaining connected paths.
	//
	// Inputs:
	//		epsilon = how close 2 points must be to be considered equal
	// Outputs:
	//		return = number of segments removed
	//

	 //think what i want is:

	// for each path
	//		for each segment in this path
	//			scan all other paths to see if it's a duplicated (within epsilon)
	//			if dup
	//				delete
	//				then re-start process because we've messed up the table of paths???

	int deleteCtr = 0;		// return value

	Logger::Write(__FUNCTION__, "Merging paths...");

	bool processing = true;
	while (processing)
	{
		processing = false;
		PointD curPos(0.0, 0.0);
		for (int pi = 0; pi < GetElementCount(); ++pi)
		{
			ElemDesc* pElem = &m_elemList[pi];
			if (pElem->type == ElemType::Path)
			{
				for (int si = 0; si < static_cast<int>(pElem->path.size()) - 1; ++si)
				{
					curPos.x += pElem->path[si].x;
					curPos.y += pElem->path[si].y;
					PointD nextPos;
					nextPos.x = curPos.x + pElem->path[si + 1].x;
					nextPos.y = curPos.y + pElem->path[si + 1].y;

					// now, scan all other paths looking for match
					int elem, seg;
					if (FindPathSegment(curPos, nextPos, epsilon, elem, seg, pi))
					{
						XString name1 = pElem->id + ".auto_1";
						XString name2 = pElem->id + ".auto_2";
						DeleteSubRange(pi, si, si, name1, name2);
						++deleteCtr;
	
						Logger::Write(__FUNCTION__, "    Deleted path %d segment %d.", pi, si);

						// as long as we're finding segments to delte, 
						// keep processing
						processing = true;
						break;
					}
				}
			}
		}
	}

	Logger::Write(__FUNCTION__, "Merge paths complete.");

	// now that's the basic logic, but delete is actually a fairly complex
	// operation, often -but not always - creating multiple paths in its wake
	//
	// might be cleaner to track contiguous dups, and delete the range in one go
	// using DeleteSubRange()
	//

	mb_IsDirty = true;

	return deleteCtr;
}

/* static */ const char* SvgFile::ElemTypeToText(ElemType elemType)
{
	switch (elemType) {
	case ElemType::None: return "None";
	case ElemType::Rectangle: return "Rectangle";
	case ElemType::Circle: return "Circle";
	case ElemType::Ellipse: return "Ellipse";
	case ElemType::Line: return "Line";
	case ElemType::PolyLine: return "Polyline";
	case ElemType::Polygon: return "Polygon";
	case ElemType::Path: return "Path";
	case ElemType::Text: return "Text";
	default:
		Logger::Write(__FUNCTION__, "Unhandled element type: %d", (int)elemType);
		assert(false);
	}

	return "---";
}

/* static */ const char* SvgFile::PathCmdToText(PathCmd cmd)
{
	switch (cmd) {
	case PathCmd::None: return "None";
	case PathCmd::M: return "M";	// M = moveto, absolute
	case PathCmd::m: return "m";	// M = moveto, relative
	case PathCmd::L: return "L";	// L = lineto
	case PathCmd::l: return "l";
	case PathCmd::H: return "H";	// H = horizontal lineto
	case PathCmd::h: return "h";
	case PathCmd::V: return "V";	// V = vertical lineto
	case PathCmd::v: return "v";
	case PathCmd::C: return "C";	// C = curveto
	case PathCmd::c: return "c";
	case PathCmd::S: return "S";	// S = smooth curveto
	case PathCmd::s: return "s";
	case PathCmd::Q: return "Q";	// Q = quadratic Bézier curve
	case PathCmd::q: return "q";
	case PathCmd::T: return "T";	// T = smooth quadratic Bézier curveto
	case PathCmd::t: return "t";
	case PathCmd::A: return "A";	// A = elliptical Arc
	case PathCmd::a: return "a";
	case PathCmd::Z: return "Z";	// Z = closepath
	case PathCmd::z: return "z";
	default:
		Logger::Write(__FUNCTION__, "Unhandled path command: %d", (int)cmd);
		assert(false);
		break;
	}

	return "-";
}
