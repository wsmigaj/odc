/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file .h
///
/// @author Piotr Kuchta, ECMWF, Feb 2009

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

#include <stdlib.h>
#include <memory> 

#include "eclib/PathName.h"
#include "eclib/DataHandle.h"
#include "eclib/Log.h"

#include "odblib/DataStream.h"
#include "odblib/HashTable.h"
#include "odblib/Codec.h"
#include "odblib/HashTable.h"
#include "odblib/Column.h"
#include "odblib/MetaData.h"
#include "odblib/RowsIterator.h"
#include "odblib/HashTable.h"
#include "odblib/SQLBitfield.h"
#include "odblib/SQLAST.h"
#include "odblib/SchemaAnalyzer.h"
#include "odblib/SQLIteratorSession.h"
#include "odblib/Header.h"
#include "odblib/Reader.h"
#include "odblib/SelectIterator.h"
#include "odblib/ReaderIterator.h"
#include "odblib/Comparator.h"
#include "odblib/Decoder.h"
#include "odblib/SQLSelectFactory.h"
#include "odblib/FastODA2Request.h"
#include "odblib/MemoryBlock.h"
#include "odblib/InMemoryDataHandle.h"
#include "odblib/odb_api.h"
#include "odblib/odbcapi.h"
#include "odblib/ImportTool.h"
#include "odblib/Tool.h"
#include "odblib/TestCase.h"
#include "odblib/DateTime.h"
#include "odblib/SplitTool.h"
#include "odblib/CountTool.h"
#include "odblib/SQLTool.h"
#include "odblib/MapReduce.h"

#include "odb/TestWriteCatFiles.h"
#include "odblib/ToolFactory.h"

#include "odb/TestOdaCAPI.h"

using namespace std;
using namespace eclib;


namespace odb { namespace tool { void producer_consumer(); }}

namespace odb {
namespace tool {
namespace test {

typedef long long llong;

void foobar()
{
	Reader in("concatenated.odb");
	Reader::iterator it = in.begin();
	Reader::iterator end = in.end();

	Writer<> out("copy_of_concatenated.odb");
	Writer<>::iterator o = out.begin();
	o->pass1(it, end);

	Comparator().compare("concatenated.odb", "copy_of_concatenated.odb");
}
//TESTCASE(foobar);

void createDataForMixedAggregated()
{
	// See TestAggregateFunctions.sql as well
	const char *data = 
	"x:INTEGER,y:INTEGER,v:DOUBLE\n"
	"1,1,0.3\n"
	"1,1,0.2\n"
	"2,2,0.4\n"
	"2,2,0.1\n"
	;

	ImportTool::importText(data, "selectAggregatedAndNonAggregated.odb");
}

TEST(selectAggregatedAndNonAggregated)
{
	createDataForMixedAggregated();

	odb::Select oda("select x,min(v),max(v) from \"selectAggregatedAndNonAggregated.odb\"");
	odb::Select::iterator it = oda.begin();

	double r0 = (*it)[0], r1 = (*it)[1], r2 = (*it)[2];

	Log::info(Here()) << "selectAggregatedAndNonAggregated: " << r0 << ", " << r1 << ", " << r2 << endl;

	ASSERT(Comparator::same(r0, 1));
	ASSERT(Comparator::same(r1, 0.2));
	ASSERT(Comparator::same(r2, 0.3));

	++it;

	r0 = (*it)[0];
	r1 = (*it)[1];
	r2 = (*it)[2];

	Log::info(Here()) << "selectAggregatedAndNonAggregated: " << r0 << ", " << r1 << ", " << r2 << endl;

	//ASSERT((*it)[0] == 2 && (*it)[1] == 0.1);
	ASSERT(r0 == 2);
	ASSERT(r1 == 0.1);
	ASSERT(r2 == 0.4);
	++it;
	ASSERT( ! (it != oda.end() ));
}


void createDataForMixedAggregated2()
{
	Writer<> out("selectAggregatedAndNonAggregated2.odb");
	Writer<>::iterator o = out.begin();
	MetaData& md(o->columns());

	typedef DataStream<SameByteOrder, DataHandle> DS;
	md.addColumn<DS>("x", "INTEGER");//, true, .0);
	md.addColumn<DS>("y", "INTEGER");//, true, .0);
	md.addColumn<DS>("v", "DOUBLE");//, true, .0);
	o->writeHeader();

	for (size_t row = 0; row < 1000; ++row)
		for (size_t x = 0; x < 10; (*o)[0] = ++x)
			for (size_t y = 0; y < 10; (*o)[1] = ++y)
				for (double v = 0; v < 10; (*o)[2] = ++v)
					++o;
}

TEST(selectAggregatedAndNonAggregated2)
{
	createDataForMixedAggregated2();
	odb::Select oda("select x,min(v),y,max(v) from \"selectAggregatedAndNonAggregated2.odb\"");
	odb::Select::iterator it = oda.begin();
	unsigned long counter = 0;
	for ( ; it != oda.end(); ++it, ++counter)
	{
		//double r0 = (*it)[0], r1 = (*it)[1], r2 = (*it)[2], r3 = (*it)[3];
		//Log::info(Here()) << "selectAggregatedAndNonAggregated2: " << r0 << ", " << r1 << ", " << r2 << ", " << r3 << endl;
	}
	Log::info(Here()) << "selectAggregatedAndNonAggregated2: counter= " << counter << endl;
	ASSERT(counter == 110);
}

void createDataForMixedAggregated3()
{
	// See TestAggregateFunctions.sql as well
	const char *data = 
	"x:STRING,y:INTEGER,v:DOUBLE\n"
	"'A',1,0.3\n"
	"'A',1,0.2\n"
	"'B',2,0.4\n"
	"'B',2,0.1\n"
	;

	ImportTool::importText(data, "selectAggregatedAndNonAggregated3.odb");
}

TEST(selectAggregatedAndNonAggregated3)
{
	createDataForMixedAggregated3();
	
	odb::Select oda("select x,count(*) from \"selectAggregatedAndNonAggregated3.odb\"");
	odb::Select::iterator it = oda.begin();
	unsigned long counter = 0;
	for ( ; it != oda.end(); ++it, ++counter)
	{
		double r0 = (*it)[0], r1 = (*it)[1];
		Log::info(Here()) << "selectAggregatedAndNonAggregated3: " << r0 << ", " << r1 << endl;
	}
	Log::info(Here()) << "selectAggregatedAndNonAggregated3: counter= " << counter << endl;
	ASSERT(counter == 2);
}


void createDataForMixedAggregatedNULL()
{
	// See TestAggregateFunctions.sql as well
	const char *data = 
	"x:REAL,y:INTEGER,v:DOUBLE\n"
	"100,1,0.3\n"
	"100,1,0.2\n"
	"101,2,0.4\n"
	"101,2,0.1\n"
	"NULL,1,0.1\n"
	"NULL,2,0.2\n"
	"NULL,3,0.3\n"
	;

	ImportTool::importText(data, "selectAggregatedAndNonAggregatedNULL.odb");
}

TEST(selectAggregatedAndNonAggregatedNULL)
{
	createDataForMixedAggregatedNULL();
	
	odb::Select oda("select x,count(*) from \"selectAggregatedAndNonAggregatedNULL.odb\"");
	odb::Select::iterator it = oda.begin();
	unsigned long counter = 0;
	for ( ; it != oda.end(); ++it, ++counter)
	{
		double r0 = (*it)[0], r1 = (*it)[1];
		Log::info(Here()) << "selectAggregatedAndNonAggregatedNULL: " << r0 << ", " << r1 << endl;
	}
	Log::info(Here()) << "selectAggregatedAndNonAggregatedNULL: counter= " << counter << endl;
	ASSERT(counter == 3);
}


/////////////////////////////////////////
// Regular expressions on the select list

void createDataForRegex1()
{
	// See TestAggregateFunctions.sql as well
	const char *data = 
	"aa:INTEGER,ab:INTEGER,ba:INTEGER,bb:INTEGER\n"
	"1,2,3,4\n"
	"10,20,30,40\n"
	"11,22,33,44\n"
	;

	ImportTool::importText(data, "regex1.odb");
}

void regex1()
{
	//createDataForRegex1();
	odb::Select oda("select \"/a.*/\" from \"regex1.odb\"");
	odb::Select::iterator it = oda.begin();
	MetaData& md(it->columns());

	Log::info() << "regex1: " << md << endl;
	
	ASSERT(md.size() == 2);
}
//TESTCASE(regex1);

TEST(vector_syntax)
{

	const char *data = 
	"a:INTEGER,b:INTEGER\n"
	"1,1\n"
	"2,2\n"
	"3,3\n"
	"4,4\n"
	"5,5\n"
	"6,6\n"
	"7,7\n"
	"8,8\n"
	"9,9\n"
	"10,10\n"
	;

	ImportTool::importText(data, "vector_syntax.odb");

	const char *sql =
	"set $X = [1,2,3,4,5];"
	"select * from \"vector_syntax.odb\" where a in $X;"
	;

	odb::Select oda(sql);
	odb::Select::iterator it = oda.begin();
	odb::Select::iterator end = oda.end();
	unsigned long counter = 0;
	for ( ; it != oda.end(); ++it, ++counter)
		;
	ASSERT(counter == 5);
}


TEST(bitfieldsLength)
{
	Log::info() << "Test_bitfieldsLength: sizeof(Decoder::W)" << sizeof(Decoder::W) << endl;
	Log::info() << "Test_bitfieldsLength: sizeof(double)" << sizeof(double) << endl;

//>>> int('0b11100110011',2)
//1843
//>>> len('0b11100110011')
//13
//>>> 
	{
		stringstream s;
		Decoder::printBinary(s, 1843);
		string r = s.str();
		Log::info() << "Test_bitfieldsLength: " << r << endl;

		ASSERT(r.size() == 11);
		ASSERT(r == "11100110011");
	}
	{
		stringstream s;
		Decoder::printBinary(s, 0);
		string r = s.str();
		Log::info() << "Test_bitfieldsLength: " << r << endl;

		ASSERT(r.size() == 1);
		ASSERT(r == "0");
	}
}

void create_stringInWhere_file()
{
	const char *data = 
	"a:STRING,b:INTEGER\n"
	"'aaa',1\n"
	"'aaaa',2\n"
	"'bbb',2\n"
	"'bbbc',2\n"
	;
	ImportTool::importText(data, "stringInWhere.odb");
}

TEST(stringInWhere)
{
	create_stringInWhere_file();
	odb::Select oda("select * from 'stringInWhere.odb' where a = 'aaa'");
	
	unsigned long counter = 0;
	for (odb::Select::iterator it = oda.begin(); it != oda.end(); ++it, ++counter)
		;
	ASSERT(counter == 1);
}

TEST(vector_syntax2)
{
	const char* sql = "set $y = 100; set $x = [$y, 'a', 'b', [1, 2]];";
	odb::sql::SQLInteractiveSession session;
	odb::sql::SQLParser p;
	p.parseString(sql, static_cast<DataHandle*>(0), odb::sql::SQLSelectFactory::instance().config());

}

TEST(blocksSizes)
{
	size_t numberOfBlocks = 0;
	off64_t* offsets = 0;
	size_t* sizes = 0;

	int r = get_blocks_offsets("TestFastODA2Request2BIG.odb", &numberOfBlocks, &offsets, &sizes);
	ASSERT(r == 0);

	Log::info() << "Test_blocksSizes: num of blocks: " << numberOfBlocks << endl;
	for (size_t i = 0; i < numberOfBlocks; ++i)
	{
		Log::info() << "Test_blocksSizes: #" << i << ": offset: " << offsets[i] << ", sizes: " << sizes[i] << endl;
	}
	Log::info() << "blocksSizes: numberOfBlocks=" << numberOfBlocks << endl;
	ASSERT(numberOfBlocks == 5);
	
	release_blocks_offsets(&offsets);
	release_blocks_sizes(&sizes);
}


TEST(rownumber1)
{
	createDataForMixedAggregated2();
    string path("selectAggregatedAndNonAggregated2.odb");
    string query("SELECT rownumber() from \"" + path + "\";");

    odb::Select select(query);
    odb::Select::iterator it = select.begin();
    odb::Select::iterator end = select.end();

	llong i = 0;
    for (; it != end; ++it)
    {
        ASSERT((*it)[0] == ++i);
    }
	ASSERT(i == 1000000);
}

TEST(sqlOutputFormatting)
{
	// See TestAggregateFunctions.sql as well
	const char *data = 
	"x:REAL,y:INTEGER,v:DOUBLE\n"
	"100,1,0.3\n"
	"100,1,0.2\n"
	"101,2,0.4\n"
	"101,2,0.1\n"
	"NULL,1,0.1\n"
	"NULL,2,0.2\n"
	"NULL,3,0.3\n"
	;

	const char* testFile("sqlOutputFormatting.odb");

	ImportTool::importText(data, testFile);

	bool doNotWriteColumnNames(false); // -T
	bool doNotWriteNULL(false);        // -N
	string delimiter(" ");           // -delimiter
	string inputFile(testFile);           // -i
	string outputFile;          // -o
	string outputFormat;        // default is ascii

	odb::sql::SQLSelectFactory::instance()
		.config(odb::sql::SQLOutputConfig(doNotWriteColumnNames, doNotWriteNULL, delimiter, outputFile, outputFormat));

	ostream& out = cout;
	odb::sql::SQLInteractiveSession session(out);
	odb::sql::SQLParser p;

	FileHandle fh(inputFile);
	fh.openForRead();
	//p.parseString(StringTool::readFile(fileName), &fh, odb::sql::SQLSelectFactory::instance().config());
	p.parseString("select x,y,v", &fh, odb::sql::SQLSelectFactory::instance().config());

}

double julian(double d, double t)
{
    int indate = (int) d;
    int intime = (int) t;
    int year_target = indate/10000;
    int month_target = (indate%10000)/100;
    int day_target = indate%100;
    int hour_target = intime/10000;
    int min_target = (intime%10000)/100;
    int sec_target = intime%100;

    utils::DateTime d1(year_target, month_target, day_target,
                   hour_target, min_target, sec_target);

    return d1.dateToJulian();
}

TEST(dateTime)
{
	int j1 = julian(20120714, 120000);
	int j2 = julian(20120714, 0);
	//ASSERT(j1 > j2);
}

void createDataForWindSpeedWindDirection()
{
	const char *data = 
	"u:REAL,v:REAL\n"
	"11.7,-5.8\n"
	"0.0,0.0\n"
	"0,5.4\n"
	"5.4,0.0\n"
	;

	ImportTool::importText(data, "uv.odb");
}

TEST(windSpeedWindDirection)
{
	createDataForWindSpeedWindDirection();
    string path("uv.odb");
    string query("SELECT ff(u,v), dd(u,v), speed(u,v),dir(u,v), sqrt(u*u+v*v), fmod(atan2(-u,-v)+360.,360.) from \"" + path + "\";");

    odb::Select select(query);
    odb::Select::iterator it = select.begin();
    odb::Select::iterator end = select.end();

	llong i = 0;
    for (; it != end; ++it)
    {
        Log::info() << " ff = " << (*it)[0] << " speed sqrt= " << (*it)[4] << endl;
        Log::info() << " dd = " << (*it)[1] << " direction atan= " << (*it)[5] << endl;
        ASSERT((*it)[0] == (*it)[4]);
        ASSERT((*it)[1] == (*it)[5]);
    }
}

TEST(odbcapi)
{
	odb::tool::test::test_odacapi_setup_in_C(0,0);
	odb::tool::test::test_odacapi3(0,0);
}

TEST(HashTable_clone)
{
	using namespace odb::codec;
	odb::codec::HashTable *h = new odb::codec::HashTable;
	h->store("BUFRDATA");
	h->store("OTHER_ST");

	odb::codec::HashTable *c = h->clone();

	//ASSERT(*h == *c);
	delete h;
	delete c;
}

void SplitTool_chunks()
{
	const char * fn = "selectAggregatedAndNonAggregated.odb";
	unsigned long long n = CountTool::fastRowCount(fn);
	vector<pair<Offset,Length> > chunks = SplitTool::getChunks(fn);

	Log::info() << "chunks.size():" << chunks.size() << endl;
	ASSERT(chunks.size() == 1 && chunks[0].first == Offset(0) && chunks[0].second == Length(357));
}
//TESTCASE(SplitTool_chunks);


void FilePool1()
{
	//FilePool<SimpleFilePoolTraits> pool(1);
}
//TESTCASE(FilePool1);

void copyVectorToArray()
{
	const size_t size = 1024; 
	const size_t n = 1000000;

	vector<double> v(size);
	double a[size];

	{ 
		Timer timer("std::copy");
		for (size_t i = 0; i < n; ++i)
			std::copy(v.begin(), v.end(), a);
	}

	{
		Timer timer("for loop");
		for (size_t i = 0; i < n; ++i)
			for (size_t j = 0; j < size; ++j)
				a[j] = v[j];
	}

}
//TESTCASE(copyVectorToArray);


void count(void *counter, const double* data, size_t n) { ++*((llong*)counter); }
void *create_counter()
{
	llong* r = new llong;
	*r = 0;
	return r;
}
void destroy_counter(void *counter) { delete (llong*) counter; }
void *reduce_counter(void *left, void *right)
{
	llong *result = new llong;
	*result = (*(llong *) left) + (*(llong *) right);
	return result;
}
CallBackProcessOneRow create_counter_callback() 
{
	CallBackProcessOneRow cb;
	cb.mapper = count;
	cb.reducer = reduce_counter;
	cb.create = create_counter;
	cb.destroy = destroy_counter;
	return cb;
}


void map_reduce_mt()
{
    const string fileName = "/scratch/ma/mak/odb-16/all.odb";
    const string sql = "select lat,lon";
	llong n = CountTool::fastRowCount(fileName);
	llong* result = (llong*) MultipleThreadMapReduce::process(0, fileName, sql, create_counter_callback());
	Log::info() << "map_reduce: MultipleThreadMapReduce::process => " << *result << endl;
	ASSERT(*result == n);
}
//TESTCASE(map_reduce_mt);

void map_reduce_st()
{
    const string fileName = "/scratch/ma/mak/odb-16/all.odb";
    const string sql = "select lat,lon";
	llong n = CountTool::fastRowCount(fileName);
	llong r = 0;
	llong* result = (llong*) SingleThreadMapReduce::process(&r, fileName, sql, create_counter_callback());
	Log::info() << "map_reduce: SingleThreadMapReduce::process => " << *result << endl;
	ASSERT(*result == n);
	//delete result;
}
//TESTCASE(map_reduce_st);


///////////////////////////////////////////////////////////////////////

void array_count(void *counter, struct Array a)
{
	*((llong*) counter) += a.nRows;

	for (size_t i = 0; i < a.nRows; ++i)
	{
		for (size_t j = 0; j < a.nCols; ++j)
		{
			//double x = *( ((double *) a.data) + i * a.nRows + j);
		}
	}
}

CallBackProcessArray create_array_counter_callback()
{
	CallBackProcessArray cb;
	cb.mapper = array_count;
	cb.reducer = reduce_counter;
	cb.create = create_counter;
	cb.destroy = destroy_counter;
	return cb;
}

void process_array_st()
{
	//llong* result = (llong*) SingleThreadMapReduce::process(0, fileName, sql, create_array_counter_callback());
	//Log::info() << "map_reduce: SingleThreadMapReduce::process=> " << *result << endl;
    const string fileName = "/scratch/ma/mak/odb-16/all.odb";
    const string sql = "select lat,lon";
	
	llong* result = (llong*) SingleThreadMapReduce::process(0, fileName, sql, create_array_counter_callback());
	Log::info() << "map_reduce: MultipleThreadMapReduce::process=> " << *result << endl;
}
//TESTCASE(process_array_st);


void process_array_mt()
{
	//llong* result = (llong*) SingleThreadMapReduce::process(0, fileName, sql, create_array_counter_callback());
	//Log::info() << "map_reduce: SingleThreadMapReduce::process=> " << *result << endl;
	
    const string fileName = "/scratch/ma/mak/odb-16/all.odb";
    const string sql = "select lat,lon";
	llong* result = (llong*) MultipleThreadMapReduce::process(0, fileName, sql, create_array_counter_callback());
	Log::info() << "map_reduce: MultipleThreadMapReduce::process=> " << *result << endl;
}
//TESTCASE(process_array_mt);


void producer_consumer()
{
	odb::tool::producer_consumer();
}
//TESTCASE(producer_consumer);


class TemporaryPathName : public PathName {
public:
	 TemporaryPathName(const string &fn) : PathName (fn) {}
	 ~TemporaryPathName() { unlink(); }
};

typedef TemporaryPathName ScratchFile;

TEST(hash_operator_on_select_list)
{
	const char *data = 
	"x:INTEGER,y:INTEGER\n"
	"1,1\n"
	"2,2\n"
	"3,3\n"
	"4,4\n"
	"5,5\n"
	"6,6\n"
	"7,7\n"
	"8,8\n"
	"9,9\n"
	"10,10\n"
	;

	ScratchFile f("hash_operator.odb");
	ImportTool::importText(data, f);

	string sql("select x,x#-1,x#1 from \"" + f + "\"");
    odb::Select select(sql);
    odb::Select::iterator it = select.begin();
    odb::Select::iterator end = select.end();
    for (; it != end; ++it)
    {
		Log::info() << it << endl;
	}

}


TEST(hash_operator_in_where)
{
	const char *data = 
	"x:INTEGER,y:INTEGER\n"
	"1,1\n"
	"2,2\n"
	"3,3\n"
	"4,4\n"
	"5,5\n"
	"6,6\n"
	"7,7\n"
	"8,8\n"
	"9,9\n"
	"10,10\n"
	;

	ScratchFile f("hash_operator.odb");
	ImportTool::importText(data, f);

	string sql("select x,x#-1,x#1 from \"" + f + "\" where x=2 and x#1=3");
    odb::Select select(sql);
    odb::Select::iterator it = select.begin();
    odb::Select::iterator end = select.end();
    for (; it != end; ++it)
    {
		Log::info() << it << endl;
	}
}

TEST(bitfields_hash_operator)
{
	PathName f("2000010106.4.0.odb");
    //odb::Select select("select lat,lat#1 from \"" + f + "\"");
    odb::Select select("select anflag@body,anflag.final@body,anflag.*@body from \"" + f + "\"");
    odb::Select::iterator it = select.begin();
    odb::Select::iterator end = select.end();
    for (; it != end; ++it)
    {
		Log::info() << it << endl;
	}
	
}

TEST(select_constant_value)
{
	const char *sql =
	"set $foo = 27;"
	"select $foo;"
	;

	odb::Select o(sql);

	odb::Select::iterator it = o.begin();
	odb::Select::iterator end = o.end();
	unsigned long counter = 0;
	for ( ; it != o.end(); ++it, ++counter)
	{
		Log::info() << it << endl;
		CHECK_EQUAL(it->data(0), 27);
	}
	CHECK_EQUAL(counter, 1);
}

/* TODO:
TEST(select_variables)
{
	const char *sql =
	"set $x=1; set $v=[1,2,3]; select * from variables;"
	;
	
	unsigned long counter = 0;
	odb::Select o(sql);
	for (odb::Select::iterator it(o.begin()), end(o.end());
		it != o.end();
		++it, ++counter)
	{
		Log::info() << (*it)[0] << ", " << (*it)[1] << endl;
	}
}
*/

TEST(include)
{
	ofstream f("stuff.sql");
	f 
		//<< "select * from \"file1.odb\";" << endl
		<< "set $foo = 10;" << endl
		<< "set $bar = 20;" << endl;
	f.close();

	const char *sql =
	"#include \"stuff.sql\"\n"
	"set $baz = $bar;"
	"select $foo * $bar;"
	;
	
	unsigned long counter = 0;
	odb::Select o(sql);
	for (odb::Select::iterator it(o.begin()), end(o.end());
		it != o.end();
		++it, ++counter)
	{
		Log::info() << it << endl;
	}
}

TEST(log_error)
{
    Log::error() << "Just a logger test" << endl;
}

/*
TEST(create_table_using_variable)
{

	const char *sql =
    "SET $c = { 2 : \"foo\" };\n"
    "SET $s = 2;\n"
    "CREATE TABLE t AS (c[$s]);"
	;
	
	unsigned long counter = 0;
	odb::Select o(sql);
	for (odb::Select::iterator it(o.begin()), end(o.end());
		it != o.end();
		++it, ++counter)
	{
		Log::info() << it << endl;
	}
}
*/

typedef MetaDataReader<MetaDataReaderIterator> MDR;

TEST(meta_data_reader_checks_if_file_truncated)
{
	ASSERT(0 == system("dd if=disp.7.1.odb of=disp.7.1.odb.truncated bs=1914000 count=1"));
	MDR mdr("disp.7.1.odb.truncated");
	try {
		for(MDR::iterator it(mdr.begin()), end(mdr.end()); it != end; ++it)
			;
		ASSERT(0 && "Scanning of truncated file did not fail");
	} catch (ShortFile ex) {
		Log::info() << "Scanning of truncated file disp.7.1.odb.truncated failed as expected." << endl;
	}
}

TEST(meta_data_reader_fails_scanning_corrupted_file)
{
	ASSERT(0 == system("cat disp.7.1.odb disp.7.1.odb.truncated >corrupted.odb"));
	MDR mdr("corrupted.odb");
	try {
		for(MDR::iterator it(mdr.begin()), end(mdr.end()); it != end; ++it)
			;
		ASSERT(0 && "Scanning of corrupted.odb did not fail");
	} catch (ShortFile ex) {
		Log::info() << "Scanning of corrupted.odb failed as expected." << endl;
	}
}

TEST(operator_ge)
{
	const char *data = 
	"a:INTEGER,b:INTEGER\n"
	"1,1\n"
	"2,2\n"
	"3,3\n"
	"4,4\n"
	"5,5\n"
	"6,6\n"
	"7,7\n"
	"8,8\n"
	"9,9\n"
	"10,10\n"
	;

	ImportTool::importText(data, "1to10.odb");

	odb::Select odb("select a,b from \"1to10.odb\" where a >= 3;");
	unsigned long counter = 0;
	for (odb::Select::iterator it = odb.begin(), end = odb.end();
        it != end;
        ++it, ++counter) 
        ;
	ASSERT(counter == 8);
}

void create_1to10()
{
	const char *data = 
	"a:INTEGER,b:INTEGER\n"
	"1,1\n"
	"2,2\n"
	"3,3\n"
	"4,4\n"
	"5,5\n"
	"6,6\n"
	"7,7\n"
	"8,8\n"
	"9,9\n"
	"10,10\n"
	;

	ImportTool::importText(data, "1to10.odb");
}

/* FIXME
TEST(Select_isNewDataset)
{
    create_1to10();
    size_t blocks (0);

	odb::Select odb("select * from \"1to10.odb\";");
	for (odb::Select::iterator it = odb.begin(), end = odb.end();
        it != end;
        ++it)
        if (it->isNewDataset())
            ++blocks;
    ASSERT(blocks == 1);
}
*/

template<typename T> 
void test_isNewDataset()
{
    create_1to10();
    size_t blocks (0);

    blocks = 0;
    T odb("1to10.odb");
    for (typename T::iterator it = odb.begin(), end = odb.end();
        it != end;
        ++it)
        if (it->isNewDataset())
            ++blocks;
    ASSERT(blocks == 1);
}

/*
TEST(Gabor)
{
    const char * cfg = 
    "CLASS: class\n"
    "DATE: andate\n"
    "TIME: antime\n"
    "TYPE: type\n"
    "OBSGROUP: groupid\n"
    "REPORTYPE: reportype\n"
    "STREAM: stream\n"
    "EXPVER: expver\n"
    ;
    const string fileName("/tmp/gabor/massaged.odb");

    odb::FastODA2Request<odb::ODA2RequestClientTraits> o2r;
    o2r.parseConfig(cfg);

    eclib::OffsetList offsets;
    eclib::LengthList lengths;
    vector<ODAHandle*> handles;
    bool rc = o2r.scanFile(fileName, offsets, lengths, handles);
    for (size_t i = 0; i < handles.size(); ++i)
        delete handles[i];
    handles.clear();
    ASSERT(rc);

    ASSERT(lengths.size());
    ASSERT(lengths.size() == offsets.size());
    for(size_t i = 1; i < offsets.size(); i++)
        ASSERT(offsets[i] > offsets[i-1]);
    size_t last = offsets.size()-1;
    ASSERT(PathName(fileName).size() == offsets[last] + lengths[last]);

    unsigned long long cnt = o2r.rowsNumber();

    string filesRequest = "ARCHIVE,\n";
    filesRequest += o2r.genRequest();
}
// FIXME
TEST(Reader_isNewDataset) { test_isNewDataset<Reader>(); }
// FIXME
TEST(MetaDataReader_isNewDataset) { test_isNewDataset<odb::MetaDataReader<MetaDataReaderIterator> >(); }

TEST(create_temporary_table)
{
    const char* sql = "CREATE "
                      " TEMPORARY " 
                      " TABLE foo AS (col1 pk9real, col2 pk9real,) INHERITS (bar,baz)";

    cout << "Trying to execute: '" << sql << "'" << endl;

	odb::Select o(sql);
    odb::tool::SQLTool::execute(sql);

}
*/


} // namespace test 
} // namespace tool 
} // namespace odb 

