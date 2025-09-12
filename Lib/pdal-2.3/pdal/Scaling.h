#ifndef SCALING_H
#define SCALING_H

#include <pdal/PointView.h>
#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	class ProgramArgs;

	/**
	  Scaling provides support for transforming X/Y/Z values from double to
	  scaled integers and vice versa.
	*/
	class Scaling
	{
	public:
		XForm m_xXform;          ///< X-dimension transform (scale/offset)
		XForm m_yXform;          ///< Y-dimension transform (scale/offset)
		XForm m_zXform;          ///< Z-dimension transform (scale/offset)
		Arg* m_xOffArg;
		Arg* m_yOffArg;
		Arg* m_zOffArg;
		Arg* m_xScaleArg;
		Arg* m_yScaleArg;
		Arg* m_zScaleArg;

		/**
		   Determine if any of the transformations are non-standard.

		   \return  Whether any transforms are non-standard.
		*/
		bool nonstandard() const
		{
			return m_xXform.nonstandard() || m_yXform.nonstandard() ||
				m_zXform.nonstandard();
		}

		/**
		  Compute an automatic scale/offset for points in the PointView.

		  \param view  PointView on which scale should be computed.
		*/
		virtual void setAutoXForm(const PointViewSet& pvSet);

		/**
		  Add option/command-line arguments for transform variables.

		  \param args  Argument set to add to.
		*/
		void addArgs(ProgramArgs& args);
	};
}

#endif // #ifndef SCALING_H
