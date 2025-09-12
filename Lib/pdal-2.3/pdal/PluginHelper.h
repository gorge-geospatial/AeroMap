/******************************************************************************
* Copyright (c) 2012, Howard Butler (hobu.inc@gmail.com)
****************************************************************************/

#pragma once

#include <string>

#include <pdal/pdal_export.h>

#include <pdal/PluginManager.h>
#include <pdal/PluginInfo.h>
#include <pdal/Kernel.h>

//DEPRECATED
#define CREATE_SHARED_PLUGIN(version_major, version_minor, T, type, info) \
    extern "C" void PF_initPlugin() \
    { \
        bool stage = std::is_convertible<T*, Stage *>::value; \
        if (stage) \
            pdal::PluginManager<pdal::Stage>::registerPlugin<T>(info); \
        else \
            pdal::PluginManager<pdal::Kernel>::registerPlugin<T>(info); \
    }

#define CREATE_SHARED_KERNEL(T, info) \
    extern "C" void PF_initPlugin() \
    { pdal::PluginManager<pdal::Kernel>::registerPlugin<T>(info); }

#define CREATE_SHARED_STAGE(T, info) \
    extern "C" void PF_initPlugin() \
    { pdal::PluginManager<pdal::Stage>::registerPlugin<T>(info); }

#define CREATE_STATIC_KERNEL(T, info) \
    static bool T ## _b = \
        pdal::PluginManager<pdal::Kernel>::registerPlugin<T>(info);

#define CREATE_STATIC_STAGE(T, info) \
    static bool T ## _b =  \
        pdal::PluginManager<pdal::Stage>::registerPlugin<T>(info);

