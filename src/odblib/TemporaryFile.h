/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File TemporaryFile.h
// Baudouin Raoult - ECMWF Sep 01

#ifndef TemporaryFile_H
#define TemporaryFile_H

#include "eclib/NonCopyable.h"
#include "eclib/filesystem/PathName.h"

class TemporaryFile : public  eckit::PathName,
                      private eckit::NonCopyable {
public:

    TemporaryFile();
	~TemporaryFile(); // Change to virtual if base class

};

#endif
