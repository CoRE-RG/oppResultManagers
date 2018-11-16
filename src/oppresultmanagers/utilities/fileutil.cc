//==========================================================================
//  FILEUTIL.CC - part of
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//  Author: Andras Varga
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2006-2015 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "oppresultmanagers/utilities/fileutil.h"

#include "omnetpp.h"

#ifdef _WIN32
#include <direct.h>
#include <cstdlib>  // _MAX_PATH
#else
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <cstring>
#include <cerrno>
#include <string>




void splitFileName(const char *pathname, std::string& dir, std::string& fnameonly)
{
    if (!pathname || !*pathname) {
        dir = ".";
        fnameonly = "";
        return;
    }

    // find last "/" or "\"
    const char *s = pathname + strlen(pathname) - 1;
    s--;  // ignore potential trailing "/"
    while (s > pathname && *s != '\\' && *s != '/')
        s--;
    const char *sep = s <= pathname ? nullptr : s;

    // split along that
    if (!sep) {
        // no slash or colon
        if (strchr(pathname, ':') || strcmp(pathname, ".") == 0 || strcmp(pathname, "..") == 0) {
            fnameonly = "";
            dir = pathname;
        }
        else {
            fnameonly = pathname;
            dir = ".";
        }
    }
    else {
        fnameonly = s+1;
        dir = std::string(pathname, static_cast<size_t>(s-pathname+1));
    }
}

std::string directoryOf(const char *pathname)
{
    std::string dir, dummy;
    splitFileName(pathname, dir, dummy);
    return dir;
}

void removeFile(const char *fname, const char *descr)
{
    if (unlink(fname) != 0 && errno != ENOENT)
        throw omnetpp::cRuntimeError("cannot remove %s `%s': %s", descr, fname, strerror(errno));
}

void mkPath(const char *pathname)
{
    if (!fileExists(pathname)) {
        std::string pathprefix, dummy;
        splitFileName(pathname, pathprefix, dummy);
        mkPath(pathprefix.c_str());
        // note: anomaly with slash-terminated dirnames: stat("foo/") says
        // it does not exist, and mkdir("foo/") says cannot create (EEXIST):
#ifdef _WIN32
        if (mkdir(pathname) != 0 && errno != EEXIST)
#else
        if (mkdir(pathname, ACCESSPERMS) != 0 && errno != EEXIST)
#endif
            throw omnetpp::cRuntimeError("cannot create directory `%s': %s", pathname, strerror(errno));
    }
}

bool fileExists(const char *pathname)
{
    // Note: stat("foo/") ==> error, even when "foo" exists and is a directory!
    struct stat statbuf;
    return stat(pathname, &statbuf) == 0;
}
