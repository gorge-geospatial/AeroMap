/******************************************************************************
* Copyright (c) 2020, Hobu Inc.
****************************************************************************/

#include <pdal/Log.h>

#include <cpl_conv.h>
#include <cpl_error.h>

namespace pdal
{
	namespace gdal
	{
		// This is a little confusing because we have a singleton error handler with
		// a single log pointer, but we set the log pointer/debug state as if we
		// were taking advantage of GDAL's thread-specific error handing.
		//
		// We lock the log/debug so that it doesn't
		// get changed while another thread is using or setting.
		class ErrorHandler
		{
		public:
			ErrorHandler();
			~ErrorHandler();

			/**
			  Get the singleton error handler.

			  \return  Reference to the error handler.
			*/
			static ErrorHandler& getGlobalErrorHandler();

			/**
			  Set the log and debug state of the error handler.  This is
			  a convenience and is equivalent to calling setLog() and setDebug().

			  \param log  Log to write to.
			  \param doDebug  Debug state of the error handler.
			*/
			void set(LogPtr log, bool doDebug);


			/**
			  Clear an error handler.
			*/
			void clear();

			/**
			  Set the log to which error/debug messages should be written.

			  \param log  Log to write to.
			*/
			void setLog(LogPtr log);

			/**
			  Set the debug state of the error handler.  Setting to true will also
			  set the environment variable CPL_DEBUG to "ON".  This will force GDAL
			  to emit debug error messages which will be logged by this handler.

			  \param doDebug  Whether we're setting or clearing the debug state.
			*/
			void setDebug(bool doDebug);

			/**
			  Get the last error and clear the error last error value.

			  \return  The last error number.
			*/
			int errorNum();

			/**
			  Handle error messages from GDAL.
			*/
			void handle(int level, int num, const char* msg);

		private:
			std::mutex m_mutex;
			bool m_debug;
			pdal::LogPtr m_log;
			mutable int m_errorNum;
			bool m_cplSet;
			CPLErrorHandler m_prevHandler;
		};

		class ErrorHandlerSuspender
		{
		public:
			ErrorHandlerSuspender();
			~ErrorHandlerSuspender();
		};
	}
}
