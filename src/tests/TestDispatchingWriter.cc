/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file UnitTest.h
///
/// @author Piotr Kuchta, ECMWF, June 2009

#include "tools/CountTool.h"
#include "odblib/DispatchingWriter.h"
#include "odblib/Reader.h"
#include "odblib/StringTool.h"

#include "eckit/testing/UnitTest.h"

using namespace std;
using namespace eckit;
using namespace odb;

/// UnitTest DispatchingWriter
///
static void test()
{
	const string fileName = "2000010106.odb";

	odb::Reader oda(fileName);
	odb::Reader::iterator it = oda.begin();
	const odb::Reader::iterator end = oda.end();
	ASSERT(it->columns().size() > 0);

	odb::DispatchingWriter writer("disp.{obstype}.{sensor}.odb");
	odb::DispatchingWriter::iterator wi = writer.begin();
	unsigned long long n1 = wi->pass1(it, end);
	wi->close();
	
	unsigned long long sum = 0;
	vector<PathName> files = (**wi).getFiles();
	for (size_t i = 0; i < files.size(); ++i)
	{
        unsigned long long n = odb::tool::CountTool::rowCount(files[i]);
		Log::info() << i << ". " << files[i] << ": " << n << std::endl;
		sum += n;
	}

	ASSERT(n1 == sum);
    odb::StringTool::shell("ls -l disp.*.*.odb", Here());
}


static void setUp(){}
static void tearDown(){}

RUN_SIMPLE_TEST