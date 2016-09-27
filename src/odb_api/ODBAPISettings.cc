/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <unistd.h>

#include "eckit/config/Resource.h"
#ifdef HAVE_AIO_H    
# include "eckit/io/AIOHandle.h"
#endif
#include "eckit/io/FileHandle.h"
#include "eckit/thread/ThreadSingleton.h"
#include "eckit/parser/StringTools.h"

#include "odb_api/ODBAPISettings.h"

using namespace eckit;
using namespace std;

inline size_t MEGA(size_t n) { return n*1024*1204; }

template class eckit::ThreadSingleton<odb::ODBAPISettings>;
static ThreadSingleton<odb::ODBAPISettings> instance_;

bool odb::ODBAPISettings::debug = false;

void odb::ODBAPISettings::setHome(const char *argv0)
{
    const char* env(getenv("ODB_API_HOME"));
    if (env) {
        home_ = env;
        Log::info() << "ODB_API_HOME set to " << home_ << endl;
    } else {
        string full;
        if (argv0[0] == '/') {
            char *absoluteArgv0;
            // The resolved_path == NULL feature, not standardized in POSIX.1-2001, but standardized in POSIX.1-2008
            if(0 == (absoluteArgv0 = ::realpath(argv0, 0)))
                throw eckit::FailedSystemCall(string("realpath ") + argv0);
            full = string(absoluteArgv0);
            ::free(absoluteArgv0);
        } else if (argv0[0] == '.' && argv0[1] == '/')
        {
            size_t bufferLen =1024*8;
            char buffer[bufferLen];
            full = string( ::getcwd(buffer, bufferLen) ) + string(argv0 + 1);
        } else
        {
            vector<string> ps(StringTools::split(":", getenv("PATH")));
            for (size_t i(0); i < ps.size(); ++i)
            {
                // TODO: perhaps we should also check if the file is readable, executable, etc...
                if (PathName(ps[i] + "/" + argv0).exists())
                {
                    full = ps[i] + "/" + argv0;
                    if (ps[i][0] != '/') {
                        size_t bufferLen =1024*8;
                        char buffer[bufferLen];
                        full = string( ::getcwd(buffer, bufferLen) ) + full;
                    }
                    break;
                }
            }
            
        }
        vector<string> ps(StringTools::split("/", full));
        Log::debug() << "ODBAPISettings::setHome: argv0: " << ps << endl;
        ASSERT("odb executable should be in a bin directory" && ps.size() >= 2 && ps[ps.size() - 2] == "bin");
        ps.pop_back(); // odb
        ps.pop_back(); // bin
        home_ = "/" + StringTools::join("/", ps);
        Log::info() << "ODB_API_HOME inferred as " << home_ << endl;
    }
}

string odb::ODBAPISettings::fileInHome(const string& fileName)
{
    ASSERT(fileName[0] == '~');
    ASSERT(fileName[1] == '/');
    return home_ + fileName.substr(1);
}

void debugMeNow() {
	Log::info() << "Debug me now" << endl;
	odb::ODBAPISettings::debug = true;
}

namespace odb {

ODBAPISettings& ODBAPISettings::instance()
{
	ASSERT( &instance_.instance() != 0 );
	return instance_.instance();
}

ODBAPISettings::ODBAPISettings()
: headerBufferSize_(Resource<long>("$ODB_HEADER_BUFFER_SIZE;-headerBufferSize;headerBufferSize", MEGA(4))),
  setvbufferSize_(Resource<long>("$ODB_SETVBUFFER_SIZE;-setvbufferSize;setvbufferSize", MEGA(8))),
  useAIO_(Resource<bool>("$ODB_API_USE_AIO", false))
{}

size_t ODBAPISettings::headerBufferSize() { return headerBufferSize_; }
void ODBAPISettings::headerBufferSize(size_t n) { headerBufferSize_ = n; }

size_t ODBAPISettings::setvbufferSize() { return setvbufferSize_; }
void ODBAPISettings::setvbufferSize(size_t n) { setvbufferSize_ = n; }

void ODBAPISettings::createDirectories(const PathName& path)
{
    vector<string> parts (StringTools::split("/", path));
    if (parts.size() < 2)
        return;

    parts.pop_back();
    PathName directory ((string(path)[0] == '/' ? "/" : "") + StringTools::join("/", parts));

    Log::debug() << "Making sure diretory " << directory << " exists" << endl;

    directory.mkdir();
}

DataHandle* ODBAPISettings::writeToFile(const PathName& fn, const Length& length, bool openDataHandle)
{
    // ODB-122 Create subdirectories before creating a file
    createDirectories(fn);

#ifdef HAVE_AIO_H    
	DataHandle* h (useAIO_
                    ? static_cast<DataHandle*>(new AIOHandle(fn))
                    : static_cast<DataHandle*>(new FileHandle(fn)));
#else
	DataHandle* h (static_cast<DataHandle*>(new FileHandle(fn)));
#endif
	if (openDataHandle) h->openForWrite(length);
	return h;
}

DataHandle* ODBAPISettings::appendToFile(const PathName& fn, const Length& length, bool openDataHandle)
{
#ifdef HAVE_AIO_H    
	DataHandle *h (useAIO_ 
                    ? static_cast<DataHandle*>(new AIOHandle(fn))
                    : static_cast<DataHandle*>(new FileHandle(fn)));
#else
	DataHandle* h (static_cast<DataHandle*>(new FileHandle(fn)));
#endif
	if (openDataHandle) h->openForAppend(length);
	return h;
}

} // namespace odb
