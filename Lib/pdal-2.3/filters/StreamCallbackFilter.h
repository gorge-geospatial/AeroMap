#ifndef STREAMCALLBACKFILTER_H
#define STREAMCALLBACKFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

#include <functional>

namespace pdal
{
	class StreamCallbackFilter : public Filter, public Streamable
	{
	public:
		std::string getName() const
		{
			return "filters.streamcallback";
		}

		StreamCallbackFilter()
		{}

		typedef std::function<bool(PointRef&)> CallbackFunc;
		void setCallback(CallbackFunc cb)
		{
			m_callback = cb;
		}

	private:
		virtual void filter(PointView& view)
		{
			PointRef p(view, 0);
			for (PointId idx = 0; idx < view.size(); ++idx)
			{
				p.setPointId(idx);
				processOne(p);
			}
		}

		virtual bool processOne(PointRef& point)
		{
			if (m_callback)
				return m_callback(point);
			return false;
		}

		CallbackFunc m_callback;

		StreamCallbackFilter& operator=(const StreamCallbackFilter&); // not implemented
		StreamCallbackFilter(const StreamCallbackFilter&); // not implemented
	};
}

#endif // #ifndef STREAMCALLBACKFILTER_H
