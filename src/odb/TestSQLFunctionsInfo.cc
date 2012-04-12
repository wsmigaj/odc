/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file TestSQLFunctionsInfo.h
///
/// @author Piotr Kuchta, ECMWF, Feb 2009

#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include "eclib/Log.h"
#include "odblib/Tool.h"
#include "odblib/TestCase.h"
#include "odblib/FunctionExpression.h"
#include "TestSQLFunctionsInfo.h"
#include "odblib/ToolFactory.h"

namespace odb {
namespace tool {
namespace test {

ToolFactory<TestSQLFunctionsInfo> _TestSQLFunctionsInfo("TestSQLFunctionsInfo");

TestSQLFunctionsInfo::TestSQLFunctionsInfo(int argc, char **argv)
: TestCase(argc, argv)
{}

void TestSQLFunctionsInfo::test()
{
	typedef vector<pair<string, int> > FI;

	FI& functionsInfo = odb::sql::expression::function::FunctionFactory::functionsInfo();

	Log::info() << "FunctionFactory::functionsInfo().size() == " << functionsInfo.size() << endl;
	for (FI::iterator i = functionsInfo.begin(); i != functionsInfo.end(); ++i)
	{
		Log::info() << i->first << "/" << i->second;
		if (i + 1 != functionsInfo.end())
			Log::info() << ", ";
	}
	Log::info() << endl;
}

} // namespace test
} // namespace tool 
} // namespace odb 

