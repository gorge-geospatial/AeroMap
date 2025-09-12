/******************************************************************************
* Copyright (c) 2015, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

// The DynamicLibrary was modeled very closely after the work of Gigi Sayfan in
// the Dr. Dobbs article:
// http://www.drdobbs.com/cpp/building-your-own-plugin-framework-part/206503957
// The original work was released under the Apache License v2.

#pragma once

#include <string>

namespace pdal
{
	class DynamicLibrary
	{
	public:
		static DynamicLibrary* load(const std::string& path, std::string& errorString);

		~DynamicLibrary();
		void* getSymbol(const std::string& name);
		void clear();

	private:
		DynamicLibrary(void* handle) 
			: m_handle(handle)
		{
		}

		DynamicLibrary();  // Unimplemented
		DynamicLibrary(const DynamicLibrary&);  // Unimplemented

	private:
		void* m_handle;
	};
}
