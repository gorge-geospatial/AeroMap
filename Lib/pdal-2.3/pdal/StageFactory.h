/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
****************************************************************************/

#pragma once

#include <pdal/pdal_export.h>
#include <pdal/Stage.h>

#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace pdal
{
	class Options;

	/**
	  This class provides a mechanism for creating Stage objects given a driver
	  name.  Creates stages are owned by the factory and destroyed when the
	  factory is destroyed.  Stages can be explicitly destroyed with destroyStage()
	  if desired.

	  \note  Stage creation is thread-safe.
	*/
	class StageFactory
	{
	public:
		/**
		  Create a stage factory.

		  \param ignored  Ignored argument.
		*/
		StageFactory(bool ignored = true);

		/**
		  Infer the reader to use based on a filename.

		  \param filename  Filename that should be analyzed to determine a driver.
		  \return  Driver name or empty string if no reader can be inferred from
			the filename.
		*/
		static std::string inferReaderDriver(const std::string& filename);

		/**
		  Infer the writer to use based on filename extension.

		  \return  Driver name or empty string if no writer can be inferred from
			the filename.
		*/
		static std::string inferWriterDriver(const std::string& filename);

		/**
		  Create a stage and return a pointer to the created stage.
		  The factory takes ownership of any successfully created stage.

		  \param stage_name  Type of stage to by created.
		  \return  Pointer to created stage.
		*/
		Stage* createStage(const std::string& type);

		/**
		  Destroy a stage created by this factory.  This doesn't need to be
		  called unless you specifically want to destroy a stage as all stages
		  are destroyed when the factory is destroyed.

		  \param stage  Pointer to stage to destroy.
		*/
		void destroyStage(Stage* stage);

	private:
		StageFactory& operator=(const StageFactory&); // not implemented
		StageFactory(const StageFactory&); // not implemented

		std::vector<std::unique_ptr<Stage>> m_ownedStages;
		std::mutex m_mutex;
	};
}
