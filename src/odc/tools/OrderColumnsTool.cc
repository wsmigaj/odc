/*
 * (C) Copyright 2021 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/FileHandle.h"
#include "eckit/log/Log.h"
//#include "odc/Comparator.h"
#include "odc/api/Odb.h"
#include "odc/Reader.h"
#include "odc/Writer.h"
#include "odc/tools/OrderColumnsTool.h"

#include <numeric>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace eckit;

namespace odc {
namespace tool {

OrderColumnsTool::OrderColumnsTool (int argc, char *argv[]) : Tool(argc, argv) { }

void OrderColumnsTool::run()
{
  if (parameters().size() != 3)
  {
    Log::error() << "Usage: ";
    usage(parameters(0), Log::error());
    Log::error() << std::endl;
    throw UserError("Expected exactly 3 command line parameters");
  }

  api::Settings::treatIntegersAsDoubles(false);
  api::Settings::setDoubleMissingValue(2147483647.0);

  PathName inFile = parameters(1);
  PathName outFile = parameters(2);

  odc::Reader in(inFile);

  std::vector<std::string> columnNames;
  std::vector<size_t> numValueChangesInColumn;
  std::vector<double> previousRow, currentRow;
  size_t numNewDatasets = 1;
  for (odc::Reader::iterator begin(in.begin()), it(begin), end(in.end()); it != end; ++it) {
    if (it->isNewDataset())
      ++numNewDatasets;
    const size_t numColumns = it->columns().size();
    if (numValueChangesInColumn.size() < numColumns) {
      ASSERT_MSG(numValueChangesInColumn.empty(),
                 "Frames with different numbers of columns aren't supported yet");
      // And changing order of columns isn't supported either...
      for (size_t c = numValueChangesInColumn.size(); c < numColumns; ++c)
        columnNames.push_back(it->columns().at(c)->name());
      numValueChangesInColumn.resize(numColumns, 0);
    }

    currentRow.clear();
    for (size_t col = 0; col != numColumns; ++col)
      currentRow.push_back(it->data(col));

    if (previousRow.size() == currentRow.size())
      for (size_t col = 0; col != numColumns; ++col)
        if (currentRow[col] != previousRow[col])
          ++numValueChangesInColumn[col];

    currentRow.swap(previousRow);
  }

  std::vector<size_t> optimallyOrderedColumnIndices(numValueChangesInColumn.size());
  std::iota(optimallyOrderedColumnIndices.begin(), optimallyOrderedColumnIndices.end(), 0);
  std::stable_sort(optimallyOrderedColumnIndices.begin(), optimallyOrderedColumnIndices.end(),
                   [&](size_t colA, size_t colB)
                   { return numValueChangesInColumn[colA] < numValueChangesInColumn[colB]; });

  std::cout << "Number of new datasets: " << numNewDatasets << std::endl;
  std::cout << "Number of value changes in each column:\n";
  for (size_t col = 0; col != numValueChangesInColumn.size(); ++col)
    std::cout << col << ": " << numValueChangesInColumn[col] << "\n";

  std::cout << "Columns with the least changes:\n";
  for (size_t col = 0; col != optimallyOrderedColumnIndices.size(); ++col)
    std::cout << col << ": " << optimallyOrderedColumnIndices[col] << "\n";

  std::vector<std::string> optimallyOrderedColumnNames;
  for (size_t colIndex : optimallyOrderedColumnIndices)
    optimallyOrderedColumnNames.push_back(columnNames[colIndex]);

  {
    FileHandle inHandle(inFile);
    inHandle.openForRead();
    AutoClose inCloser(inHandle);

    FileHandle outHandle(outFile);
    const Length length;
    outHandle.openForWrite(length);
    AutoClose outCloser(outHandle);

    api::Reader in(inHandle, false);

    api::Frame frame;

    std::vector<std::vector<double>> data;
    std::vector<api::StridedData> strides;

    std::vector<std::string> readColumnNames;
    std::vector<api::StridedData> readData;

    std::vector<api::ColumnInfo> writtenColumns;
    std::vector<api::ConstStridedData> writtenData;

    while ((frame = in.next())) {

      const std::vector<api::ColumnInfo> &readColumns = frame.columnInfo();
      const size_t rowCount = frame.rowCount();

      readColumnNames.clear();
      for (const api::ColumnInfo &column : readColumns)
        readColumnNames.push_back(column.name);

      if (data.size() < readColumns.size())
        data.resize(readColumns.size());
      readData.clear();
      for (size_t col = 0; col < readColumns.size(); ++col) {
        data[col].resize(rowCount);
        readData.emplace_back(data[col].data(), rowCount, sizeof(double), sizeof(double));
      }

      api::Decoder decoder(readColumnNames, readData);
      decoder.decode(frame);

      writtenColumns.clear();
      writtenData.clear();
      for (const std::string &columnName : optimallyOrderedColumnNames) {
        const auto columnIt = std::find_if(readColumns.begin(), readColumns.end(),
                                           [columnName](const api::ColumnInfo &ci)
                                           { return ci.name == columnName; });
        if (columnIt != readColumns.end()) {
          size_t readColumnIndex = columnIt - readColumns.begin();
          writtenColumns.push_back(*columnIt);
          const api::StridedData &data = readData[readColumnIndex];
          writtenData.emplace_back(*data, data.nelem(), data.dataSize(), data.stride());
        }
      }
      api::encode(outHandle, writtenColumns, writtenData, {}, -1);
    }
  }



//  FileHandle fh(inFile);
//  fh.openForRead();
//  AutoClose closer(fh);

//  std::unordered_map<std::string, size_t> numValueChangesByColumn;

//  {
//    api::Reader in(fh);

//    api::Frame frame;

//    std::vector<std::vector<char>> data;
//    std::vector<api::StridedData> strides;

//    while ((frame = in.next())) {

//      //    std::cout << "Row count: " << frame.rowCount() << std::endl;
//      //    std::cout << "Column count: " << frame.columnCount() << std::endl << std::endl;
//      size_t nrows = frame.rowCount();

//      if (data.size() < frame.columnCount())
//        data.resize(frame.columnCount());

//      size_t iCol = 0;
//      for (const auto & column : frame.columnInfo()) {
//        data[iCol].resize(nrows * sizeof(double));
//        strides.emplace_back()
//        ++iCol;
//      }

//      uint64_t data0[nrows];
//      uint64_t data1[nrows];
//      double data2[nrows];
//      char data3[nrows][16];
//      double data4[nrows];

//      std::vector<std::string> columns {
//          "column0",
//          "column1",
//          "column2",
//          "column3",
//          "column4",
//      };

//      std::vector<api::StridedData> strides {
//          // ptr, nrows, element_size, stride
//          {data0, nrows, sizeof(uint64_t), sizeof(uint64_t)},
//          {data1, nrows, sizeof(uint64_t), sizeof(uint64_t)},
//          {data2, nrows, sizeof(double), sizeof(double)},
//          {data3, nrows, 16, 16}, // column3 is a 16-byte string column
//          {data4, nrows, sizeof(double), sizeof(double)},
//      };

//      api::Decoder decoder(columns, strides);
//      decoder.decode(frame);

//      std::cout << "Column " << i++ << std::endl;
//      std::cout << "  name: " << column.name << std::endl;
//      std::cout << "  type: " << columnTypeName(column.type) << std::endl;
//      std::cout << "  size: " << column.decodedSize << std::endl;

//      int j = 0;
//      if (column.type == BITFIELD) {
//        for (auto const& bf : column.bitfield) {
//          std::cout << "  bitfield " << j++ << std::endl;
//          std::cout << "      name: " << bf.name << std::endl;
//          std::cout << "      offset: " << bf.offset << std::endl;
//          std::cout << "      nbits: " << bf.size << std::endl;
//        }
//      }
//    }



//	odc::Writer<> out(outFile);

//	odc::Reader::iterator it(in.begin());
//	odc::Reader::iterator end(in.end());

//	odc::Writer<>::iterator writer(out.begin());
//	writer->pass1(it, end);
	
//	odc::Reader outReader(outFile);
//	Log::info() << "Verifying." << std::endl;
//	odc::Reader::iterator it1 = in.begin();
//	odc::Reader::iterator end1 = in.end();

//	odc::Reader::iterator it2 = outReader.begin();
//	odc::Reader::iterator end2 = outReader.end();

//	odc::Comparator comparator;
//	comparator.compare(it1, end1, it2, end2, inFile, outFile);
}

} // namespace tool 
} // namespace odc 

