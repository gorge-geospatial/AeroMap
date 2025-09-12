/******************************************************************************
* Copyright (c) 2020, Hobu Inc.
****************************************************************************/

#include <mutex>

#include "ErrorHandler.h"

namespace pdal
{
	namespace gdal
	{
		/**
		  Return a reference to the global error handler.

		  \return  Reference to the global error handler.
		*/
		ErrorHandler& ErrorHandler::getGlobalErrorHandler()
		{
			static ErrorHandler s_gdalErrorHandler;

			return s_gdalErrorHandler;
		}

		namespace 
		{

			//ABELL - No idea why this is __stdcall
			void __stdcall trampoline(::CPLErr code, int num, char const* msg)
			{
				ErrorHandler::getGlobalErrorHandler().handle((int)code, num, msg);
			}
		}

		/**
		  Constructor for a GDAL error handler.
		*/
		ErrorHandler::ErrorHandler() : m_errorNum(0), m_prevHandler(nullptr)
		{
			std::string value;

			// Will return thread-local setting
			const char* set = CPLGetConfigOption("CPL_DEBUG", "");
			m_cplSet = (bool)set;
			m_debug = m_cplSet;
		}


		/**
		  Destructor for a GDAL error handler.
		*/
		ErrorHandler::~ErrorHandler()
		{}


		/**
		  Set the output destination and debug for the error handler.
		  \param log  Pointer to logger.
		  \param debug  Whether GDAL debugging should be turned on.
		*/
		void ErrorHandler::set(LogPtr log, bool debug)
		{
			// Set an error handler
			if (m_prevHandler == nullptr)
				m_prevHandler = CPLSetErrorHandler(&trampoline);
			setLog(log);
			setDebug(debug);
		}


		/**
		  Clear the PDAL error handler.
		*/
		void ErrorHandler::clear()
		{
			CPLSetErrorHandler(m_prevHandler);
			m_prevHandler = nullptr;
		}


		/**
		  Set the log destination for GDAL errors.
		  \param log  Pointer to the logger.
		*/
		void ErrorHandler::setLog(LogPtr log)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_log = log;
		}

		/**
		  Set the log state for GDAL logging.
		  \param debug  If true, sets the CPL_DEBUG logging option for GDAL
		*/
		void ErrorHandler::setDebug(bool debug)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_debug = debug;

			if (debug)
				CPLSetThreadLocalConfigOption("CPL_DEBUG", "ON");
			else
				CPLSetThreadLocalConfigOption("CPL_DEBUG", NULL);
		}


		/**
		  Get the number of the last GDAL error.
		  \return  Last GDAL error number.
		*/
		int ErrorHandler::errorNum()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_errorNum;
		}

		/**
		  Callback for GDAL error.
		  \param level  Error level
		  \param num  Error number
		  \param msg  Error message.
		*/
		void ErrorHandler::handle(int level, int num, char const* msg)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			std::ostringstream oss;

			m_errorNum = num;
			if (level == CE_Failure || level == CE_Fatal)
			{
				oss << "GDAL failure (" << num << ") " << msg;
				if (m_log)
					m_log->get(LogLevel::Error) << oss.str() << std::endl;
			}
			else if (m_debug && level == CE_Debug)
			{
				oss << "GDAL debug: " << msg;
				if (m_log)
					m_log->get(LogLevel::Debug) << oss.str() << std::endl;
			}
		}

		ErrorHandlerSuspender::ErrorHandlerSuspender()
		{
			CPLPushErrorHandler(CPLQuietErrorHandler);
		}


		ErrorHandlerSuspender::~ErrorHandlerSuspender()
		{
			(void)CPLPopErrorHandler();
		}
	}
}
