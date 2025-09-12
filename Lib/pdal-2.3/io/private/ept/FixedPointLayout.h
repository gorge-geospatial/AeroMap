#ifndef FIXEDPOINTLAYOUT_H
#define FIXEDPOINTLAYOUT_H

#include <pdal/PointTable.h>

namespace pdal
{
	class FixedPointLayout : public PointLayout
	{
		// The default PointLayout class may reorder dimension entries for packing
		// efficiency.  However if a PointLayout is intended to be mapped to data
		// coming from a remote source, then the dimensions must retain their order.
		// FixedPointLayout retains the order of dimensions as they are registered,
		// as well as adding a custom assignment function that makes sure XYZ are
		// registered as their appropriate remote type rather than always as
		// doubles.
	public:
		void registerFixedDim(Dimension::Id id, Dimension::Type type);
		Dimension::Id registerOrAssignFixedDim(std::string name, Dimension::Type type);

	protected:
		virtual bool update(pdal::Dimension::Detail dimDetail, const std::string& name) override;
	};
}

#endif // #ifndef FIXEDPOINTLAYOUT_H
