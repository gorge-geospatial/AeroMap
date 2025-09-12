/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
****************************************************************************/

#pragma once

// This file is for all the things that basically everybody has to include.

#include <iostream>

#include <pdal/pdal_export.h>
#include <pdal/pdal_types.h>

// This is an extraction from the gtest header.  It allows us to make test
// functions friends of classes without including the gtest headers.
#ifndef FRIEND_TEST
#define FRIEND_TEST(test_case_name, test_name)\
friend class test_case_name##_##test_name##_Test
#endif

// See http://stackoverflow.com/questions/1814548/boostsystem-category-defined-but-not-used
#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED 1
#endif


#define PDAL_CURRENT_BOOST_MINOR_VERSION BOOST_VERSION/100%1000
#ifdef __cplusplus
#  define PDAL_C_START           extern "C" {
#  define PDAL_C_END             }
#else
#  define PDAL_C_START
#  define PDAL_C_END
#endif


#pragma warning(disable: 4068)  // ignore unknown pragmas (due to boost's use of GCC pragmas)
#pragma warning(disable: 4250)  // ignore inherit via dominance
