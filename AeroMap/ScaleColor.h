#ifndef SCALECOLOR_H
#define SCALECOLOR_H

#include <vector>

#include "MarkTypes.h"
#include "MarkLib.h"
#include "XString.h"
#include "Logger.h"
#include "Calc.h"

class ScaleColor
{
public:

	// construction / destruction

	ScaleColor(const char* fileName = nullptr);
	virtual ~ScaleColor();

	bool Save(const char* fileName = nullptr);
	void Create(int bandCount = ScaleColor::GetDefaultBandCount());

	UInt16    AddBand(UInt8 Height, UInt8 R, UInt8 G, UInt8 B);
	PixelType GetColorByIndex(UInt16 colorIdx);            // get color where for index between 0 - GetColorCount()
	PixelType GetColorByHeight(double heightPercent);      // get color for height between 0.0 to 1.0
	void      RemoveAllBands();
	UInt16    GetBandCount() const;
	void      SetBandCount(UInt16 bandCount);
	UInt16    GetColorCount() const;
	XString   GetFileName() const;

	void SetWaterColor(PixelType color);
	void ClearWaterColor();
	bool HasWaterColor();
	PixelType GetWaterColor();

	const char* GetDescription() const;
	void SetDescription(const char* desc);

	// band-indexed methods

	UInt8 GetBandR(unsigned int bandIdx) const { return bandIdx < mv_Band.size() ? mv_Band[bandIdx].R : 0; }
	UInt8 GetBandG(unsigned int bandIdx) const { return bandIdx < mv_Band.size() ? mv_Band[bandIdx].G : 0; }
	UInt8 GetBandB(unsigned int bandIdx) const { return bandIdx < mv_Band.size() ? mv_Band[bandIdx].B : 0; }
	UInt8 GetBandHeight(unsigned int bandIdx) const { return bandIdx < mv_Band.size() ? mv_Band[bandIdx].Height : 0; }

	void SetBandColor(unsigned int bandIdx, UInt8 R, UInt8 G, UInt8 B);
	void SetBandHeight(unsigned int bandIdx, UInt8 height);

	static const char* GetDefaultExt() { return "cscale"; }
	static int GetMinBandCount();
	static int GetMaxBandCount();
	static int GetDefaultBandCount();

	static PixelType GetDefaultWaterColor();

private:

	XString ms_FileName;	// color scale file name
	XString ms_Description;

	struct BandType		    // color band definition
	{
		UInt8 R;			// color
		UInt8 G;
		UInt8 B;
		UInt8 Height;		// height, percentage 0-100

		BandType()			// constructor
		{
			R = G = B = 0;
			Height = 0;
		}
	};

	std::vector<BandType> mv_Band;		// band definitions
	std::vector<PixelType> mv_Scale;	// populated color values

	bool mb_HasWaterColor;
	PixelType m_pixWater;				// optional water color

private:

	bool Load(const char* fileName);
	void UpdateColorScale();
	int GetLowBand(unsigned int colorIndex);
	UInt8 ColorIndexToHeight(unsigned int colorIndex) const;
};

#endif // #ifndef SCALECOLOR_H
