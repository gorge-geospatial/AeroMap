#ifndef STREAMABLE_H
#define STREAMABLE_H

#include <pdal/pdal_internal.h>
#include <pdal/Stage.h>

namespace pdal
{
	class StreamableWrapper;

	class Streamable : public virtual Stage
	{
		friend class StreamableWrapper;
	public:
		Streamable();

		/**
		  Execute a prepared pipeline (linked set of stages) in streaming mode.

		  This performs the action associated with the stage by executing the
		  \ref processOne function of each stage in depth first order.  Points
		  are processed up to the capacity of the provided StreamPointTable.
		  Not all stages support streaming mode and an exception will be thrown
		  when attempting to \ref execute an unsupported stage.

		  Streaming points can reduce memory consumption, but will limit access
		  to algorithms that need to operate on full point sets.

		  \param table  Streaming point table used for stage pipeline.  This must be
			the same \ref table used in the \ref prepare function.

		*/
		virtual void execute(StreamPointTable& table);
		using Stage::execute;

		/**
		  Determine if a pipeline is streamable.

		  \return Whether the pipeline is streamable.
		*/
		virtual bool pipelineStreamable() const;

	protected:
		Streamable& operator=(const Streamable&) = delete;
		Streamable(const Streamable&); // not implemented

		using SrsMap = std::map<Streamable*, SpatialReference>;

		void execute(StreamPointTable& table, std::list<Streamable*>& stages,
			SrsMap& srsMap);

		/**
		  Process a single point (streaming mode).  Implement in subclass.

		  \param point  Point to process.
		  \return  Readers return false when no more points are to be read.
			Filters return false if a point is to be filtered-out (not passed
			to subsequent stages).
		*/
		virtual bool processOne(PointRef& /*point*/) = 0;
		/**
		{
			throwStreamingError();
			return false;
		}
		**/

		/**
		  Notification that the points that will follow in processing are from
		  a spatial reference different than the previous spatial reference.

		   \param srs  New spatial reference.
		*/
		virtual void spatialReferenceChanged(const SpatialReference& /*srs*/)
		{}

		/**
		  Find the first nonstreamable stage in a pipeline.

		  \return  NULL if the pipeline is streamable, otherwise return
			a pointer to the first found stage that's not streamable.
		*/
		const Stage* findNonstreamable() const;
	};
}

#endif // #ifndef STREAMABLE_H
