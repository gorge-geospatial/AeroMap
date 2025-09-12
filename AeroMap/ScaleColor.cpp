// ScaleColor.cpp
// Manager for color scales.
// 
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//
// Sample color scale file:
// 
//			Band= 0,   0,0,0
//			Band= 20,  40,40,40
//			Band= 40,  130,130,130
//			Band= 60,  170,170,170
//			Band= 100, 255,255,255
//			Water= 10, 20, 100
//			

#include <assert.h>

#include "Logger.h"
#include "ScaleColor.h"

#define TAG_BAND "Band="
#define TAG_DESC "Desc="
#define TAG_WATER "Water="

constexpr UInt16 DEF_BAND_COUNT = 5;		// default # of bands if none specified
constexpr UInt16 MIN_BAND_COUNT = 3;
constexpr UInt16 MAX_BAND_COUNT = 9;

constexpr UInt16 COLOR_COUNT = 1024;		// total # of colors in expanded color scale

ScaleColor::ScaleColor(const char* fileName)
	: ms_FileName("")			// color scale file name
	, mb_HasWaterColor(false)
	, m_pixWater(10, 20, 100)
{
	if (fileName == nullptr)
		Create(DEF_BAND_COUNT);
	else
		Load(fileName);
}

ScaleColor::~ScaleColor()
{
	RemoveAllBands();
}

UInt16 ScaleColor::GetBandCount() const
{
	return static_cast<UInt16>(mv_Band.size());
}

void ScaleColor::SetBandCount(UInt16 bandCount)
{
	assert(bandCount >= MIN_BAND_COUNT && bandCount <= MAX_BAND_COUNT);

	if (bandCount == GetBandCount())
		return;

	// add if needed
	BandType band;
	while (GetBandCount() < bandCount)
		mv_Band.push_back(band);

	// remove extras
	if (GetBandCount() > bandCount)
		mv_Band.resize(bandCount);

	// redisribute spacing
	for (int i = 0; i < GetBandCount(); ++i)
	{
		mv_Band[i].Height = (UInt8)(((double)i / (double)(GetBandCount() - 1)) * 100.0);
	}
}

int ScaleColor::GetMinBandCount()
{
	return MIN_BAND_COUNT;
}

int ScaleColor::GetMaxBandCount()
{
	return MAX_BAND_COUNT;
}

int ScaleColor::GetDefaultBandCount()
{
	return DEF_BAND_COUNT;
}

UInt16 ScaleColor::GetColorCount() const
{
	return (UInt16)mv_Scale.size();
}

XString ScaleColor::GetFileName() const
{
	return ms_FileName;
}

const char* ScaleColor::GetDescription() const
{
	return ms_Description;
}

void ScaleColor::SetDescription(const char* desc)
{
	ms_Description = desc;
	ms_Description.Trim();
}

PixelType ScaleColor::GetColorByIndex(UInt16 colorIdx)
{
	// get color based on index
	//
	// input:
	//		colorIdx = color table index
	//
	// output:
	//		return = color table entry at ai_ColorIdx
	//

	PixelType pix;
	if (mv_Scale.size() > 0)
	{
		if (colorIdx >= mv_Scale.size())
			pix = mv_Scale[mv_Scale.size() - 1];
		else
			pix = mv_Scale[colorIdx];
	}
	return pix;
}

PixelType ScaleColor::GetColorByHeight(double heightPercent)
{
	// Get color based on index
	//
	// Input:
	//		heightPercent = 0.0-1.0
	//
	// Output:
	//		return = RGB color table entry corresponding to heightPercent
	//

	PixelType pix;
	if (heightPercent == 0.0 && HasWaterColor())
	{
		pix = m_pixWater;
	}
	else
	{
		if (mv_Scale.size() > 0)
		{
			unsigned int colorIndex = (int)(heightPercent * GetColorCount());
			if (colorIndex >= mv_Scale.size())
				pix = mv_Scale[mv_Scale.size() - 1];
			else
				pix = mv_Scale[colorIndex];
		}
	}
	return pix;
}

bool ScaleColor::Load(const char* fileName)
{
	// load a color scale file
	//

	FILE* pInputFile = fopen(fileName, "rt");
	if (!pInputFile)
	{
		Logger::Write(__FUNCTION__, "Unable to open '%s'", fileName);
		return false;
	}

	mv_Band.clear();
	mv_Scale.clear();

	char buf[255];
	while (!feof(pInputFile))
	{
		memset(buf, 0, sizeof(buf));              // init buffer for potential eof condition
		fgets(buf, sizeof(buf), pInputFile);

		// process whether eof encountered or not - fgets() will leave buffer unchanged if it didn't read anything
		XString line = buf;
		line.TrimLeft();
		line.Replace("\n", "");
		line.Replace("\r", "");
		line.Replace("\t", "");

		if (line.BeginsWithNoCase(TAG_BAND))
		{
			// Band= 40, 130, 130, 130
			line = line.Mid(sizeof(TAG_BAND) - 1);
			line.Replace("\t", "");				// strip tabs & spaces
			line.Replace(" ", "");

			BandType band;
			int tokenCount = line.Tokenize(",");
			if (tokenCount >= 4)	// valid range definition
			{
				band.Height = line.GetToken(0).GetInt();
				band.R = line.GetToken(1).GetInt();
				band.G = line.GetToken(2).GetInt();
				band.B = line.GetToken(3).GetInt();
				mv_Band.push_back(band);
			}
		}
		else if (line.BeginsWithNoCase(TAG_WATER))
		{
			line = line.Mid(sizeof(TAG_WATER) - 1);
			line.Replace("\t", "");				// strip tabs & spaces
			line.Replace(" ", "");

			int tokenCount = line.Tokenize(",");
			if (tokenCount >= 3)
			{
				m_pixWater.R = line.GetToken(0).GetInt();
				m_pixWater.G = line.GetToken(1).GetInt();
				m_pixWater.B = line.GetToken(2).GetInt();

				mb_HasWaterColor = true;
			}
		}
		else if (line.BeginsWithNoCase(TAG_DESC))
		{
			ms_Description = line.Mid(sizeof(TAG_DESC) - 1);
			ms_Description.Trim();
		}
	}
	fclose(pInputFile);

	ms_FileName = fileName;

	UpdateColorScale();		// color scale dependent on mv_Band

	return true;
}

bool ScaleColor::Save(const char* fileName /* = "" */)
{
	// Save color scale file.
	//

	XString outputFile = ms_FileName;
	if (fileName && strlen(fileName) > 0)
		outputFile = fileName;

	FILE* pOutFile = fopen(outputFile.c_str(), "wt");
	if (!pOutFile)
	{
		Logger::Write(__FUNCTION__, "Unable to open '%s'", outputFile.c_str());
		return false;
	}

	fprintf(pOutFile, "%s%s\n", TAG_DESC, ms_Description.c_str());

	for (unsigned int bandIdx = 0; bandIdx < mv_Band.size(); ++bandIdx)
	{
		fprintf(pOutFile, "%s %d, %d,%d,%d\n",
			TAG_BAND,
			mv_Band[bandIdx].Height,
			mv_Band[bandIdx].R, mv_Band[bandIdx].G, mv_Band[bandIdx].B);
	}
	if (HasWaterColor() == true)
	{
		fprintf(pOutFile, "%s %d,%d,%d\n",
			TAG_WATER,
			m_pixWater.R, m_pixWater.G, m_pixWater.B);
	}
	fprintf(pOutFile, "\n");
	fclose(pOutFile);
	ms_FileName = fileName;

	return true;
}

void ScaleColor::Create(int bandCount)
{
	// Create a new color scale.
	//

	assert(bandCount >= MIN_BAND_COUNT && bandCount <= MAX_BAND_COUNT);

	// clear current scale
	RemoveAllBands();

	// create a solid white scale of equally spaced bands
	double delta = 1.0 / static_cast<double>(bandCount - 1);
	for (int i = 0; i < bandCount; ++i)
	{
		UInt8 height = static_cast<UInt8>((static_cast<double>(i) * delta) * 100.0);
		AddBand(height, 0xFF, 0xFF, 0xFF);
	}
}

void ScaleColor::UpdateColorScale()
{
	// using mv_Band table as source, populate mv_Scale
	// color table with all color values in output scale
	//

	if (mv_Band.size() < 2)
		return;

	mv_Scale.clear();
	for (unsigned int colorIndex = 0; colorIndex < COLOR_COUNT; ++colorIndex)
	{
		PixelType pix;      // output color for this color index

		int lowBand = GetLowBand(colorIndex);
		if (lowBand > -1)
		{
			int highBand = lowBand + 1;

			UInt8 height = ColorIndexToHeight(colorIndex);
			PixelType pix0(mv_Band[lowBand].R, mv_Band[lowBand].G, mv_Band[lowBand].B);
			PixelType pix1(mv_Band[highBand].R, mv_Band[highBand].G, mv_Band[highBand].B);
			// interpolate based on height relative to overlapping region
			double t = (double)(height - mv_Band[lowBand].Height) / (double)(mv_Band[highBand].Height - mv_Band[lowBand].Height);
			pix = pix0.Lerp(t, pix1);
			mv_Scale.push_back(pix);
		}
	}
}

int ScaleColor::GetLowBand(unsigned int colorIndex)
{
	// For a color index between 0 and COLOR_COUNT, return the bounding
	// low band for the associated height (the high band is return value + 1).
	//
	// inputs:
	//		colorIndex = colorIndex, 0 <= COLOR_COUNT
	//
	// outputs:
	//		return =
	//

	int lowBand = -1;        // return value
	UInt8 height = ColorIndexToHeight(colorIndex);

	for (unsigned int bandIdx = 0; bandIdx < mv_Band.size() - 1; ++bandIdx)
	{
		if (height >= mv_Band[bandIdx].Height && height <= mv_Band[bandIdx + 1].Height)
		{
			lowBand = bandIdx;
			break;
		}
	}
	return lowBand;
}

UInt16 ScaleColor::AddBand(UInt8 Height, UInt8 R, UInt8 G, UInt8 B)
{
	BandType band;
	band.R = R;
	band.G = G;
	band.B = B;
	band.Height = Height;
	mv_Band.push_back(band);

	UpdateColorScale();			// color scale dependent on bands

	return (UInt16)mv_Band.size();		// return # of bands
}

void ScaleColor::SetBandColor(unsigned int bandIdx, UInt8 R, UInt8 G, UInt8 B)
{
	if (bandIdx < mv_Band.size())
	{
		mv_Band[bandIdx].R = R;
		mv_Band[bandIdx].G = G;
		mv_Band[bandIdx].B = B;
		UpdateColorScale();			// color scale dependent on bands
	}
}

void ScaleColor::SetBandHeight(unsigned int bandIdx, UInt8 height)
{
	if (height > 100)
		height = 100;

	if (bandIdx < mv_Band.size())
	{
		mv_Band[bandIdx].Height = height;
		UpdateColorScale();			// color scale dependent on bands
	}
}

void ScaleColor::RemoveAllBands()
{
	// Delete all color scale data elements, leaving only header info.
	//

	mv_Band.clear();
	mv_Scale.clear();
}

UInt8 ScaleColor::ColorIndexToHeight(unsigned int colorIndex) const
{
	// Return height associated with colorIndex as a percent of maximum height.
	//

	double factor = static_cast<double>(colorIndex) / static_cast<double>((COLOR_COUNT - 1));
	return static_cast<UInt8>((factor * 100.0));
}

void ScaleColor::SetWaterColor(PixelType color)
{
	m_pixWater = color;
	mb_HasWaterColor = true;
}

void ScaleColor::ClearWaterColor()
{
	mb_HasWaterColor = false;
}

bool ScaleColor::HasWaterColor()
{
	return mb_HasWaterColor;
}

PixelType ScaleColor::GetWaterColor()
{
	return m_pixWater;
}

PixelType ScaleColor::GetDefaultWaterColor()
{
	return PixelType(30, 50, 100);
}
