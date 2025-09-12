#ifndef SRSBOUNDS_H
#define SRSBOUNDS_H

#include <pdal/SpatialReference.h>
#include <pdal/util/Bounds.h>

namespace pdal
{
	class SrsBounds : public Bounds
	{
	public:
		SrsBounds()
		{}

		explicit SrsBounds(const BOX3D& box);
		explicit SrsBounds(const BOX3D& box, const SpatialReference& srs);
		explicit SrsBounds(const BOX2D& box);
		explicit SrsBounds(const BOX2D& box, const SpatialReference& srs);

		void parse(const std::string& s, std::string::size_type& pos);
		SpatialReference spatialReference()
		{
			return m_srs;
		}

		friend std::ostream& operator << (std::ostream& out,
			const SrsBounds& bounds);

	private:
		SpatialReference m_srs;
	};

	namespace Utils
	{
		template<>
		inline StatusWithReason fromString(const std::string& s,
			SrsBounds& srsBounds)
		{
			std::string::size_type pos(0);
			srsBounds.parse(s, pos);
			return true;
		}
	}

	std::ostream& operator << (std::ostream& out, const SrsBounds& bounds);
}

#endif // #ifndef SRSBOUNDS_H
