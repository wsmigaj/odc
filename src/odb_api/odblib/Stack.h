/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef Stack_H
#define Stack_H

#include <stack>

#include "odb_api/odblib/Environment.h"
#include "odb_api/odblib/SelectOneTable.h"

namespace odb {
namespace sql {

// Forward declarations

class Environment;

struct Stack : private std::stack<Environment*> {
	Stack();
	~Stack();

	void pushFrame(const SortedTables::iterator);
	void popFrame();

	void print(std::ostream& s) const;

	const SortedTables::iterator& tablesIterator(); 

	SelectOneTable& table();
	void table(SelectOneTable *);

	SQLTableIterator& cursor();
	void cursor(SQLTableIterator *);
};


} // namespace sql
} // namespace odb

#endif