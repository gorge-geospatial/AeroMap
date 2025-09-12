#ifndef DBREADER_H
#define DBREADER_H

#include <pdal/Reader.hpp>
#include <pdal/XMLSchema.hpp>

namespace pdal
{
	class DbReader : public Reader
	{
	protected:
		DbReader() : m_orientation(Orientation::PointMajor), m_packedPointSize(0)
		{}

		DimTypeList dbDimTypes() const;
		void loadSchema(PointLayoutPtr layout, const std::string& schemaString);
		void loadSchema(PointLayoutPtr layout, const XMLSchema& schema);
		void updateSchema(const XMLSchema& schema);
		void writeField(PointRef& point, const char* pos, const DimType& dim);
		void writePoint(PointRef& point, const char* buf);
		size_t packedPointSize() const
		{
			return m_packedPointSize;
		}
		size_t dimOffset(Dimension::Id id) const;
		Orientation orientation() const
		{
			return m_orientation;
		}

	private:
		PointLayoutPtr m_layout;
		XMLDimList m_dims;
		Orientation m_orientation;
		size_t m_packedPointSize;

		DbReader& operator=(const DbReader&); // not implemented
		DbReader(const DbReader&); // not implemented
	};
}

#endif // #ifndef DBREADER_H
