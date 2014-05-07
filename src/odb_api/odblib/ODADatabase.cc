/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/config/Resource.h"
#include "odb_api/odblib/ODADatabase.h"
#include "odb_api/odblib/Reader.h"
#include "odb_api/odblib/TextReader.h"
#include "odb_api/odblib/TextReaderIterator.h"
#include "odb_api/odblib/TODATable.h"

using namespace eckit;

namespace odb {
namespace sql {

ODADatabase::ODADatabase(const PathName& path,const std::string& name)
: SQLDatabase(path,name)
{
    setIncludePath(Resource<std::string>("$ODB_INCLUDE_PATH", ""));
}

ODADatabase::~ODADatabase() {}

void ODADatabase::open() {}

void ODADatabase::close() { SQLDatabase::close(); }

SQLTable* ODADatabase::table(const std::string& name)
{
	std::map<std::string,SQLTable*>::iterator j = tablesByName_.find(name);

    Log::info() << "ODADatabase::table(" << name << ") tablesByName_.size()==" << tablesByName_.size() << std::endl;
    for (std::map<std::string,SQLTable*>::iterator it(tablesByName_.begin()); it != tablesByName_.end(); ++it)
    {
        Log::info() << " : " << it->first << std::endl;
    }

	if(j == tablesByName_.end())
	{
		// FIXME (?): path_ is '.'. ignore now
		//tablesByName_[name] = new TODATable<Reader>(*this,path_,name);
		tablesByName_[name] = new TODATable<Reader>(*this,name,name);
		j = tablesByName_.find(name);
	}
	return (*j).second;
}

SQLTable* ODADatabase::openDataHandle(DataHandle& dh, DataFormat dataFormat)
{
	std::string name = "dataHandle@";
	//name += std::string(&dh);

	if (dataFormat == ODA) return tablesByName_[name] = new TODATable<Reader>(*this, dh);
	// TODO
	//if (dataFormat == CSV) return tablesByName_[name] = new TODATable<TextReader>(*this, dh);

	ASSERT(0 && "Format not supported");
	return 0;
}

SQLTable* ODADatabase::openDataStream(std::istream& is, const std::string& delimiter, DataFormat dataFormat) 
{
	std::string name = "dataHandle@";
	//name += std::string(&dh);

	if (dataFormat == CSV) return tablesByName_[name] = new TODATable<TextReader>(*this, is, delimiter);
	// TODO
	//if (dataFormat == CSV) return tablesByName_[name] = new TODATable<TextReader>(*this, dh);

	ASSERT(0 && "Format not supported");
	return 0;
}


} // namespace sql
} // namespace odb
