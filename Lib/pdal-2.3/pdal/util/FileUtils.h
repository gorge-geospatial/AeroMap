#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <cassert>
#include <cmath>
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <windows.h>

namespace pdal
{
	namespace FileUtils
	{
		/**
		  Open an existing file for reading.

		  \param filename  Filename.
		  \param asBinary  Read as binary file (don't convert /r/n to /n)
		  \return  Pointer to opened stream.
		*/
		std::istream* openFile(std::string const& filename,
			bool asBinary = true);

		/**
		  Create/truncate a file and open for writing.

		  \param filename  Filename.
		  \param asBinary  Write as binary file (don't convert /n to /r/n)
		  \return  Point to opened stream.
		*/
		std::ostream* createFile(std::string const& filename,
			bool asBinary = true);

		/**
		  Open an existing file for write

		  \param filename  Filename.
		  \param asBinary  Write as binary file (don't convert /n to /r/n)
		  \return  Point to opened stream.
		*/
		std::ostream* openExisting(std::string const& filename,
			bool asBinary = true);


		/**
		  Determine if a directory exists.

		  \param dirname  Name of directory.
		  \return  Whether a directory exists.
		*/
		bool directoryExists(const std::string& dirname);

		/**
		  Create a directory.

		  \param dirname  Directory name.
		  \return  Whether the directory was created.
		*/
		bool createDirectory(const std::string& dirname);

		/**
		  Create all directories in the provided path.

		  \param dirname  Path name.
		  \return  \false on failure
		*/
		bool createDirectories(const std::string& path);

		/**
		  Delete a directory and its contents.

		  \param dirname  Directory name.
		*/
		void deleteDirectory(const std::string& dirname);

		/**
		  List the contents of a directory.

		  \param dirname  Name of directory to list.
		  \return  List of entries in the directory.
		*/
		std::vector<std::string> directoryList(const std::string& dirname);

		/**
		  Close a file created with createFile.

		  \param ofs  Pointer to stream to close.
		*/
		void closeFile(std::ostream* ofs);

		/**
		  Close a file created with openFile.

		  \param ifs  Pointer to stream to close.
		*/
		void closeFile(std::istream* ifs);

		/**
		  Delete a file.

		  \param filename  Name of file to delete.
		  \return  \c true if successful, \c false otherwise
		*/
		bool deleteFile(const std::string& filename);

		/**
		  Rename a file.

		  \param dest  Desired filename.
		  \param src   Source filename.
		*/
		void renameFile(const std::string& dest, const std::string& src);

		/**
		  Determine if a file exists.

		  \param  Filename.
		  \return  Whether the file exists.
		*/
		bool fileExists(const std::string& filename);

		/**
		  Get the size of a file.

		  \param filename  Filename.
		  \return  Size of file.
		*/
		uintmax_t fileSize(const std::string& filename);

		/**
		  Read a file into a string.

		  \param filename  Filename.
		  \return  File contents as a string
		*/
		std::string readFileIntoString(const std::string& filename);

		/**
		  Get the current working directory with trailing separator.

		  \return  The current working directory.
		*/
		std::string getcwd();

		/**
		  Return the file component of the given path,
		  e.g. "d:/foo/bar/a.c" -> "a.c"

		  \param path  Path from which to extract file component.
		  \return  File part of path.
		*/
		std::string getFilename(const std::string& path);

		/**
		  Return the directory component of the given path,
		  e.g. "d:/foo/bar/a.c" -> "d:/foo/bar/"

		  \param path  Path from which to extract directory component.
		  \return  Directory part of path.
		*/
		std::string getDirectory(const std::string& path);

		/**
		  Determine if the path is an absolute path.

		  \param path  Path to test.
		  \return  Whether the path is absolute.
		*/
		bool isAbsolutePath(const std::string& path);

		/**
		  Determine if path is a directory.

		  \param path  Directory to check.
		  \return  Whether the path represents a directory.
		*/
		bool isDirectory(const std::string& path);

		/**
		  Return the path with all ".", ".." and symbolic links removed.
		  The file must exist.

		  \param filename  Name of file to convert to canonical path.
		  \return  Canonical version of provided filename, or empty string.
		*/
		std::string toCanonicalPath(std::string filename);


		/**
		  If the filename is an absolute path, just return it otherwise,
		  make it absolute (relative to current working dir) and return it.

		  \param filename  Name of file to convert to absolute path.
		  \return  Absolute version of provided filename.
		*/
		std::string toAbsolutePath(const std::string& filename);

		/**
		  If the filename is an absolute path, just return it otherwise,
		  make it absolute (relative to base dir) and return that.

		  \param filename  Name of file to convert to absolute path.
		  \param base  Base name to use.
		  \return  Absolute version of provided filename relative to base.
		*/
		std::string toAbsolutePath(const std::string& filename,
			const std::string base);

		/**
		  Get the file creation and modification times.

		  \param filename  Filename.
		  \param createTime  Pointer to creation time structure.
		  \param modTime  Pointer to modification time structure.
		*/
		void fileTimes(const std::string& filename, struct tm* createTime,
			struct tm* modTime);

		/**
		  Return the extension of the filename, including the separator (.).

		  \param path  File path from which to extract extension.
		  \return  Extension of filename.
		*/
		std::string extension(const std::string& path);

		/**
		  Return the filename stripped of the extension.  . and .. are returned
		  unchanged.

		  \param path  File path from which to extract file stem.
		  \return  Stem of filename.
		*/
		std::string stem(const std::string& path);

		/**
		  Expand a filespec to a list of files.

		  \param filespec  File specification to expand.
		  \return  List of files that correspond to provided file specification.
		*/
		std::vector<std::string> glob(std::string filespec);


		struct MapContext
		{
		public:
			MapContext() : m_fd(-1), m_addr(nullptr)
			{}

			void* addr() const
			{
				return m_addr;
			}
			std::string what() const
			{
				return m_error;
			}

			int m_fd;
			uintmax_t m_size;
			void* m_addr;
			std::string m_error;
			HANDLE m_handle;
		};
		/**
		  Map a file to memory.
		  \param filename  Filename to map.
		  \param readOnly  Must be true at this time.
		  \param pos       Starting position of file to map.
		  \param size      Number of bytes in file to map.
		  \return  MapContext.  addr() gets the mapped address.  what() gets
			 any error message.  addr() returns nullptr on error.
		*/
		MapContext mapFile(const std::string& filename, bool readOnly = true, uintmax_t pos = 0, uintmax_t size = 0);

		/**
		  Unmap a previously mapped file.
		  \param ctx  Previously returned MapContext
		  \return  MapContext indicating current state of the file mapping.
		*/
		MapContext unmapFile(MapContext ctx);
	}
}

#endif // #ifndef FILEUTILS_H
