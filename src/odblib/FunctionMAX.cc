/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "FunctionMAX.h"
#include <values.h>

namespace odb {
namespace sql {
namespace expression {
namespace function {

FunctionMAX::FunctionMAX(const string& name,const expression::Expressions& args)
: FunctionExpression(name, args),
  value_(-DBL_MAX)
{}

FunctionMAX::FunctionMAX(const FunctionMAX& other)
: FunctionExpression(other.name_, other.args_),
  value_(other.value_)
{}

SQLExpression* FunctionMAX::clone() const { return new FunctionMAX(*this); }

const odb::sql::type::SQLType* FunctionMAX::type() const { return args_[0]->type(); }

FunctionMAX::~FunctionMAX() {}

double FunctionMAX::eval(bool& missing) const
{
	if (value_ == -DBL_MAX)
		missing = true;

	return value_;
}

void FunctionMAX::prepare(SQLSelect& sql)
{
	FunctionExpression::prepare(sql);
	value_ = -DBL_MAX;
}

void FunctionMAX::output(SQLOutput& s) const 
{ 
	bool missing = false;
	double d = eval(missing); 
	type()->output(s, d, missing);
}

void FunctionMAX::cleanup(SQLSelect& sql)
{
	FunctionExpression::cleanup(sql);
	value_ = -DBL_MAX;
}

void FunctionMAX::partialResult() 
{
	bool missing = false;
	double value = args_[0]->eval(missing);
	if(!missing)
		if(value > value_)
			value_ = value;
}

static FunctionMaker<FunctionMAX> make_MAX("max",1);

} // namespace function
} // namespace expression
} // namespace sql
} // namespace odb

