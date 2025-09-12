#ifndef OGR_H
#define OGR_H

#include <string>

namespace hexer
{
	class HexGrid;
}

namespace pdal
{
	class OGR
	{
		using OGRDataSourceH = void*;
		using OGRLayerH = void*;

	public:
		OGR(std::string const& filename, const std::string& srs,
			std::string driver = "ESRI Shapefile", std::string layerName = "");
		~OGR();

		void writeBoundary(hexer::HexGrid* grid);
		void writeDensity(hexer::HexGrid* grid);

	private:
		std::string m_filename;
		std::string m_driver;

		OGRDataSourceH m_ds;
		OGRLayerH m_layer;
		std::string m_layerName;

		void createLayer(const std::string& wkt);
	};
}

#endif // #ifndef OGR_H
