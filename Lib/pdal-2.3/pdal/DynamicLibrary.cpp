/******************************************************************************
* Copyright (c) 2015, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

// The DynamicLibrary was modeled very closely after the work of Gigi Sayfan in
// the Dr. Dobbs article:
// http://www.drdobbs.com/cpp/building-your-own-plugin-framework-part/206503957
// The original work was released under the Apache License v2.

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <sstream>
#include <iostream>

#include "private/DynamicLibrary.h"

namespace pdal
{
	DynamicLibrary::~DynamicLibrary()
	{
		if (m_handle)
		{
			::FreeLibrary((HMODULE)m_handle);
		}
	}

	void DynamicLibrary::clear()
	{
		m_handle = NULL;
	}

	DynamicLibrary* DynamicLibrary::load(const std::string& name,
		std::string& errorString)
	{
		if (name.empty())
		{
			errorString = "Empty path.";
			return NULL;
		}

		void* handle = NULL;

		handle = ::LoadLibraryA(name.c_str());
		if (handle == NULL)
		{
			DWORD errorCode = ::GetLastError();
			std::stringstream ss;
			ss << std::string("LoadLibrary(") << name
				<< std::string(") Failed. errorCode: ")
				<< errorCode;
			errorString = ss.str();
		}
		return new DynamicLibrary(handle);
	}

	void* DynamicLibrary::getSymbol(const std::string& symbol)
	{
		if (!m_handle)
			return NULL;

		void* sym;
		sym = ::GetProcAddress((HMODULE)m_handle, symbol.c_str());
		return sym;
	}
}
