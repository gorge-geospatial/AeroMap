// Tool.cpp
// Abstraction of terrain editing tools.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include <math.h>
#include <assert.h>

#include "Calc.h"
#include "Tool.h"

// Dimension is always an odd number because it is the center
// point plus an equal amount on either side.
const int DEFAULT_MASK_SIZE = 11;
const double DEFAULT_DEPTH = 10.0;

Tool::Tool()
	: m_Tool(ToolType::None)
	, m_MaskSize(DEFAULT_MASK_SIZE)
	, mf_MaxDepth(DEFAULT_DEPTH)
	, mp_Tex(nullptr)
{
	memset(mf_Depth, 0, sizeof(mf_Depth));
}

Tool::~Tool()
{
	delete mp_Tex;
}

void Tool::SetTool(ToolType tool)
{
	// Set tool type.

	m_Tool = tool;

	// tool-specific setup
	switch (m_Tool) {
	case ToolType::TexPaint:
		//TODO:
		//test texture
		SetTexture("D:/Media/Textures/moss1-512.jpg");
		break;
	case ToolType::Router:
		CreateDepthMask();
		break;
	}

	emit(tool_change_sig());
}

Tool::ToolType Tool::GetTool() const
{
	return m_Tool;
}

void Tool::Clear()
{
	m_Tool = ToolType::None;

	m_MaskSize = DEFAULT_MASK_SIZE;
	mf_MaxDepth = DEFAULT_DEPTH;

	delete mp_Tex;
	mp_Tex = nullptr;
}

void Tool::SetBrushSize(int /*size*/)
{
	// Set height/width of brush for tools that 
	// support it.
	//
}

int Tool::GetBrushSize()
{
	return 0;
}

PixelType Tool::GetPixel(int row, int col)
{
	// Get texture paint pixel.
	//

	UInt16 x = (UInt16)(col % mp_Tex->GetWidth());
	UInt16 y = (UInt16)(row % mp_Tex->GetHeight());
	PixelType pix = mp_Tex->GetPixel(x, y);
	return pix;
}

void Tool::SetTexture(const char* fileName)
{
	// Set texture to be used for paiting.
	//

	delete mp_Tex;
	mp_Tex = new ImageFile(fileName);
}

double Tool::GetDepth(UInt16 row, UInt16 col)
{
	// Get cut depth at (row,col)

	assert(row < m_MaskSize && col < m_MaskSize);
	return mf_Depth[row][col];
}

void Tool::SetMaskSize(int size)
{
	// Dimension is always an odd number because it is the center
	// point plus an equal amount on either side.
	assert(size % 2 == 1);
	assert(size <= _countof(mf_Depth[0]));

	m_MaskSize = size;

	CreateDepthMask();

	emit(tool_change_sig());
}

int Tool::GetMaskSize() const
{
	return m_MaskSize;
}

void Tool::CreateDepthMask()
{
	// Create a pre-calculated mask of cut-depths
	// so caller only needs to query depth value for
	// each point.
	//
	// TODO:
	//		Because of symmetry, only 1 quadrant needs
	//		to be calculated.

	if (m_Tool != ToolType::Router)
		return;

	int mid = m_MaskSize / 2;
	double radius = (double)mid;

	for (int r = 0; r < m_MaskSize; ++r)
	{
		for (int c = 0; c < m_MaskSize; ++c)
		{
			// slow, but only happens when tool created
			double distance = sqrt((double)((r - mid)*(r - mid) + (c - mid)*(c - mid)));
			//if inside radius
			if (distance <= radius)
			{
				// how far along s-curve are we?
				double ds = distance / radius;
				double s_input = (6.0 - 12.0 * ds);
				// this gives depth from 0.0 to 1.0
				double depth = SCurve6(s_input);
				// stretch it to desired depth
				depth *= mf_MaxDepth;
				mf_Depth[r][c] = depth;
			}
			else
			{
				mf_Depth[r][c] = 0.0;
			}
		}
	}
}

void Tool::SetMaxDepth(double depth)
{
	mf_MaxDepth = depth;

	CreateDepthMask();

	emit(tool_change_sig());
}

double Tool::GetMaxDepth() const
{
	return mf_MaxDepth;
}

const char* Tool::GetName()
{
	switch (m_Tool) {
	case ToolType::None:				// no tool selected
		return "None";
	case ToolType::Select:				// selecting drawing elements / geometry
		return "Select";
	case ToolType::ViewZoom:			// zoom in/out
		return "ViewZoom";
	case ToolType::ViewRotate:			// rotate view (does not modify design)
		return "ViewRotate";
	case ToolType::ViewPan:				// pan view (does not modify design)
		return "ViewPan";
	case ToolType::Light:				// change light positions
		return "Light";
	case ToolType::Distance:			// measure linear distance
		return "Distance";
	case ToolType::Area:				// measure area
		return "Area";
	case ToolType::Volume:				// measure volume
		return "Volume";
	case ToolType::Profile:				// create path profile
		return "Profile";
	case ToolType::Excavate:			// excavate rectangular region
		return "Excavate";
	case ToolType::Router:				// cut path with router bit
		return "Router";
	case ToolType::Lake:
		return "Lake";
	case ToolType::Ocean:
		return "Ocean";
	case ToolType::WaterFill:			// fill region with water-type vertices
		return "WaterFill";
	case ToolType::TexPaint:
		return "TexPaint";
	default:
		assert(false);
	}

	return "Unknown";
}
