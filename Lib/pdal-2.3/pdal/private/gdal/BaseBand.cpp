/******************************************************************************
* Copyright (c) 2020, Hobu Inc.
****************************************************************************/

#pragma warning(push)
#pragma warning(disable: 4251)
#include <gdal.h>
#include <gdal_priv.h>
#pragma warning(pop)

#include "GdalRaster.h"

namespace pdal
{
	namespace gdal
	{
		BaseBand::BaseBand(GDALDataset* ds, int bandNum, const std::string& name)
		{
			m_band = ds->GetRasterBand(bandNum);
			if (!m_band)
				throw InvalidBand();

			if (name.size())
			{
				m_band->SetDescription(name.data());
				// We don't care about offset, but this sets the flag to indicate
				// that the metadata has changed.
				m_band->SetOffset(m_band->GetOffset(NULL) + .00001);
				m_band->SetOffset(m_band->GetOffset(NULL) - .00001);
			}
		}

		void BaseBand::totalSize(int& x, int& y)
		{
			x = m_band->GetXSize();
			y = m_band->GetYSize();
		}

		void BaseBand::blockSize(int& x, int& y)
		{
			m_band->GetBlockSize(&x, &y);
		}

		void BaseBand::readBlockBuf(int x, int y, uint8_t* buf)
		{
			if (m_band->ReadBlock(x, y, buf) != CPLE_None)
				throw CantReadBlock();
		}

		void BaseBand::writeBlockBuf(int x, int y, const uint8_t* buf)
		{
			void* v = reinterpret_cast<void*>(const_cast<uint8_t*>(buf));
			if (m_band->WriteBlock(x, y, reinterpret_cast<void*>(v)) != CPLE_None)
				throw CantWriteBlock();
		}

		void BaseBand::statistics(double* minimum, double* maximum,
			double* mean, double* stddev,
			bool approx, bool force) const
		{
			m_band->GetStatistics(approx, force, minimum, maximum, mean, stddev);
		}
	}
}
