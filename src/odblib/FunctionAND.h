/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File FunctionAND.h
// Baudouin Raoult - ECMWF Dec 03

#ifndef FunctionAND_H
#define FunctionAND_H

#include "FunctionExpression.h"

namespace odb {
namespace sql {
namespace expression {
namespace function {

class FunctionAND : public FunctionExpression {
public:
	FunctionAND(const string&, const expression::Expressions&);
	FunctionAND(const FunctionAND&);
	~FunctionAND();

	SQLExpression* clone() const;

private:
// No copy allowed
	FunctionAND& operator=(const FunctionAND&);

// -- Overridden methods
	virtual double eval(bool& missing) const;
	virtual SQLExpression* simplify(bool&);
	virtual bool andSplit(expression::Expressions&);

// -- Friends
	//friend ostream& operator<<(ostream& s,const FunctionAND& p)
	//	{ p.print(s); return s; }
};

} // namespace function
} // namespace expression 
} // namespace sql
} // namespace odb 

#endif
