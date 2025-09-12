#ifndef TOOL_H
#define TOOL_H

#include "MarkTypes.h"
#include "ImageFile.h"

#include <QObject>

class Tool : public QObject
{
	Q_OBJECT

public:

	enum class ToolType
	{
		None,			// no tool selected

		// view tools

		Select,			// selecting drawing elements / geometry

		ViewZoom,		// zoom in/out
		ViewRotate,		// rotate view (does not modify design)
		ViewPan,		// pan view (does not modify design)
		Light,			// change light position

		// edit tools

		Excavate,		// level a rectangular region
		Router,			// cut path with router
		Lake,			// create a lake
		Ocean,
		WaterFill,		// flood fill region with water-type vertices
		TexPaint,		// paint with texture

		// analysis tools

		Profile,		// profile a linear path
		Distance,		// measure linear distance
		Area,			// measure area
		Volume,			// measure volume
	};

public:

	Tool();
	~Tool();

	void SetTool(ToolType tool);
	ToolType GetTool() const;
	const char* GetName();
	void Clear();

	// texture paint properties
	void SetBrushSize(int size);
	int GetBrushSize();
	PixelType GetPixel(int row, int col);
	void SetTexture(const char* fileName);

	// router properties
	void SetMaskSize(int size);
	int GetMaskSize() const;					// mask dimension on a side, data points
	double GetDepth(UInt16 row, UInt16 col);	// get cut depth at (row,col)
	void SetMaxDepth(double depth);				// maximum cut depth
	double GetMaxDepth() const;

signals:

	void tool_change_sig();

private:

	ToolType m_Tool;
	double mf_MaxDepth;

	// texture paint properties
	ImageFile* mp_Tex;

	// router properties
	int m_MaskSize;					// size of tool mask on a side, terrain data points
	double mf_Depth[21][21];

private:

	void CreateDepthMask();
};

#endif // #ifndef TOOL_H
