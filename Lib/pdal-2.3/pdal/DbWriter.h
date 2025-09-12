#ifndef DBWRITER_H
#define DBWRITER_H

#include <pdal/Scaling.hpp>
#include <pdal/Writer.hpp>
#include <pdal/XMLSchema.hpp>

#include <string>
#include <unordered_map>

namespace pdal
{
	class PgWriter;
	class OciWriter;

	class DbWriter : public Writer
	{
	protected:
		DbWriter()
		{}

		virtual void setAutoXForm(const PointViewPtr view);
		XMLDimList dbDimTypes() const
		{
			return m_dbDims;
		}
		size_t readField(const PointView& view, char* pos, Dimension::Id id,
			PointId idx);
		size_t readPoint(const PointView& view, PointId idx, char* outbuf);
		size_t packedPointSize() const
		{
			return m_packedPointSize;
		}

		// Allows subclass access to ready() without the mess of friends.
		void doReady(PointTableRef table)
		{
			DbWriter::ready(table);
		}
		void doAddArgs(ProgramArgs& args)
		{
			DbWriter::addArgs(args);
		}

	private:
		void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual void ready(PointTableRef table);

		DimTypeList m_dimTypes;
		XMLDimList m_dbDims;
		std::unordered_map<int, DimType> m_dimMap;
		Scaling m_scaling;
		std::pair<int, int> m_xOffsets;
		std::pair<int, int> m_yOffsets;
		std::pair<int, int> m_zOffsets;

		StringList m_outputDims; ///< List of dimensions to write
		size_t m_packedPointSize; ///< Size of point data as read from PointTable.
		size_t m_dbPointSize; ///< Size of point data as written to DB
		bool m_locationScaling;

		DbWriter& operator=(const DbWriter&); // not implemented
		DbWriter(const DbWriter&); // not implemented
	};
}

#endif // #ifndef DBWRITER_H
