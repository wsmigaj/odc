/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File FunctionEQ.h
// Baudouin Raoult - ECMWF Dec 03

#ifndef FunctionEQ_H
#define FunctionEQ_H

#include "FunctionExpression.h"

namespace odb {
namespace sql {
namespace expression {
namespace function {

class FunctionEQ : public FunctionExpression {
public:
	FunctionEQ(const string&,const expression::Expressions&);
	FunctionEQ(const FunctionEQ&);
	~FunctionEQ(); // Change to virtual if base class

	static bool equal(const SQLExpression& l, const SQLExpression& r, bool& missing);
	static void trimStringInDouble(char* &p, size_t& len);

	SQLExpression* clone() const;
private:
// No copy allowed
	FunctionEQ& operator=(const FunctionEQ&);

	double tmp_;

// -- Overridden methods
	virtual double eval(bool& missing) const;
	virtual SQLExpression* simplify(bool&);
	virtual bool useIndex();
	virtual SQLIndex* getIndex(double*);

// -- Friends
	//friend ostream& operator<<(ostream& s,const FunctionEQ& p)
	//	{ p.print(s); return s; }
};

} // namespace function
} // namespace expression 
} // namespace sql
} // namespace odb

#endif
