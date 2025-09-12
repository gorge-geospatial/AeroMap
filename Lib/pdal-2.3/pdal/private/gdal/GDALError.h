/******************************************************************************
* Copyright (c) 2020, Hobu Inc.
****************************************************************************/

#pragma once

namespace pdal
{
	namespace gdal
	{
		enum class GDALError
		{
			None,
			NotOpen,
			CantOpen,
			NoData,
			InvalidBand,
			BadBand,
			NoTransform,
			NotInvertible,
			CantReadBlock,
			InvalidDriver,
			DriverNotFound,
			CantCreate,
			InvalidOption,
			CantWriteBlock,
			InvalidType
		};

		struct InvalidBand {};
		struct BadBand {};
		struct CantReadBlock {};
		struct CantWriteBlock
		{
			CantWriteBlock()
			{}

			CantWriteBlock(const std::string& w) : what(w)
			{}

			std::string what;
		};
	}
}
