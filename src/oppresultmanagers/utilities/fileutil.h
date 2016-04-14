#ifndef OPPRESULTMANAGERS_FILEUTIL_H
#define OPPRESULTMANAGERS_FILEUTIL_H

#include <string>

void splitFileName(const char *pathname, std::string& dir, std::string& fnameonly);

std::string directoryOf(const char *pathname);

void removeFile(const char *fname, const char *descr);

void mkPath(const char *pathname);

bool fileExists(const char *pathname);

#endif
