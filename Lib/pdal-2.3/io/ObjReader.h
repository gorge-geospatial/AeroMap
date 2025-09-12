#ifndef OBJREADER_H
#define OBJREADER_H

#include <istream>
#include <map>
#include <tuple>
#include <vector>

#include <pdal/Reader.h>

namespace pdal
{
	class ObjReader : public Reader
	{
	public:
		std::string getName() const;

	private:
		/**
		  Retrieve summary information for the file. NOTE - entire file must
		  be read to retrieve summary for obj files.

		  \param table  Point table being initialized.
		*/
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void ready(PointTableRef table);
		virtual void done(PointTableRef table);
		virtual point_count_t read(PointViewPtr view, point_count_t numPts);

	private:
		struct XYZW
		{
			double x;
			double y;
			double z;
			double w;
		};
		std::vector<XYZW> m_vertices;
		std::vector<XYZW> m_textureVertices;
		std::vector<XYZW> m_normalVertices;
		TriangularMesh* m_mesh;
		using VTN = std::tuple<int64_t, int64_t, int64_t>;
		std::map<VTN, PointId> m_points;
		std::istream* m_istream;
		point_count_t m_index;

		using TRI = std::array<VTN, 3>;
		using FACE = std::vector<VTN>;

		void newVertex(double x, double y, double z);
		void newVertex(double x, double y, double z, double w);
		void newTextureVertex(double x);
		void newTextureVertex(double x, double y);
		void newTextureVertex(double x, double y, double z);
		void newNormalVertex(double x, double y, double z);
		void newTriangle(PointViewPtr view, TRI tri);
		bool readFace(FACE& vertices, PointViewPtr view);
		void extractFace(StringList fields, FACE& face);
		VTN extractVertex(const std::string& vstring);
		std::vector<TRI> triangulate(FACE face);
		PointId addPoint(PointViewPtr view, VTN vertex);
	};
}

#endif // #ifndef OBJREADER_H
