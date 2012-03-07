/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "Log.h"
#include "SQLAST.h"
#include "SQLBitfield.h"
#include "SchemaAnalyzer.h"
#include "SQLUser.h"
#include "SQLServerSession.h"

namespace odb {
namespace sql {

SQLUser::SQLUser(TCPSocket& protocol):
	NetUser(protocol)
{}

SQLUser::~SQLUser() {}

void SQLUser::serve(Stream&, istream& in,ostream& out)
{
	SQLServerSession session(in,out);
	session.serve();
}

} // namespace sql
} // namespace odb
