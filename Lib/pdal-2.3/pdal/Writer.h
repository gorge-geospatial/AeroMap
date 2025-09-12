#ifndef WRITER_H
#define WRITER_H

#include <memory>

#include <pdal/Options.h>
#include <pdal/PointView.h>
#include <pdal/Stage.h>

namespace pdal
{
	class UserCallback;

	namespace expr
	{
		class ConditionalExpression;
	}

	/**
	  A Writer is a terminal stage for a PDAL pipeline.  It usually writes output
	  to a file, but this isn't a requirement.  The class provides support for
	  some operations common for producing point output.
	*/
	class Writer : public virtual Stage
	{
		friend class WriterWrapper;
		friend class DbWriter;
		friend class FlexWriter;

		struct Args;

	public:
		Writer();
		~Writer();
		Writer& operator=(const Writer&) = delete;
		Writer(const Writer&) = delete;

		/**
		  Locate template placeholder ('#') and validate filename with respect
		  to placeholder.
		*/
		static std::string::size_type
			handleFilenameTemplate(const std::string& filename);

	private:
		virtual PointViewSet run(PointViewPtr view)
		{
			PointViewSet viewSet;
			write(view);
			viewSet.insert(view);
			return viewSet;
		}
		virtual void l_addArgs(ProgramArgs& args) final;
		virtual void l_initialize(PointTableRef table);
		virtual void l_prepared(PointTableRef table) final;

		virtual const expr::ConditionalExpression* whereExpr() const;
		virtual WhereMergeMode mergeMode() const;
		/**
		  Write the point in a PointView.  This is a simplification of the
		  \ref run() interface for convenience.  Impelment in subclass if desired.
		*/
		virtual void write(const PointViewPtr /*view*/)
		{
			std::cerr << "Can't write with stage = " << getName() << "!\n";
		}

		std::unique_ptr<Args> m_args;
	};
}

#endif // #ifndef WRITER_H
