#ifndef GROUNDKERNEL_H
#define GROUNDKERNEL_H

#include <memory>
#include <string>

#include <pdal/Kernel.h>
#include <pdal/pdal_export.h>
#include <pdal/util/FileUtils.h>

namespace pdal
{
	class Options;
	class Stage;

	class GroundKernel : public Kernel
	{
		struct Args;

	public:
		std::string getName() const override;
		int execute() override;
		GroundKernel();

	private:
		virtual void addSwitches(ProgramArgs& args);

		std::unique_ptr<Args> m_args;
	};
}

#endif // #ifndef GROUNDKERNEL_H
