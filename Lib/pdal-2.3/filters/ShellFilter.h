#ifndef SHELLFILTER_H
#define SHELLFILTER_H

#include <pdal/Filter.h>

#include <vector>
#include <string>

namespace pdal
{
	class ShellFilter : public Filter
	{
	public:
		std::string getName() const override;
		virtual void addArgs(ProgramArgs& args) override;
		virtual void initialize() override;
		virtual PointViewSet run(PointViewPtr view) override;

	private:
		std::string m_command;
		std::string m_command_output;
		virtual void done(PointTableRef table) override;
	};
}

#endif // #ifndef SHELLFILTER_H
