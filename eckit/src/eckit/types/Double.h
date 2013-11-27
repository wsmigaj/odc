/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File Double.h
// Baudouin Raoult - ECMWF Dec 97

#ifndef eckit_Double_h
#define eckit_Double_h

#include "eckit/eckit.h"

//-----------------------------------------------------------------------------

namespace eckit {

//-----------------------------------------------------------------------------

class DumpLoad;

class Double {
public:

	Double(double = 0);
	Double(const string&);

#include "eckit/types/Double.b"

	~Double() {}

public: // operators

	operator string() const;

	bool operator==(const Double& other) const
		{ return round() == other.round(); }

	bool operator!=(const Double& other) const
		{ return round() != other.round(); }

	bool operator<(const Double& other) const
		{ return round() < other.round(); }

	bool operator>(const Double& other) const
		{ return round() > other.round(); }

public: // methods

	void dump(DumpLoad&) const;
	void load(DumpLoad&);

	double round() const;

protected: // methods

	void print(ostream& s) const;

private: // members

	double value_;

	friend ostream& operator<<(ostream& s,const Double& p)
		{ p.print(s); return s; }

};


//-----------------------------------------------------------------------------

} // namespace eckit

#endif