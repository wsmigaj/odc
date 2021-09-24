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
#include "odc/api/Odb.h"
#include "odc/tools/OrderColumnsTool.h"

#include <numeric>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace eckit;

namespace odc {
namespace tool {

namespace {

std::vector<std::string> findOptimalColumnOrder(const PathName &inFile) {
  std::vector<std::string> columnNamesInOriginalOrder;
  std::unordered_map<std::string, size_t> numValueChangesInColumn;

  {
    FileHandle inHandle(inFile);
    inHandle.openForRead();
    AutoClose inCloser(inHandle);

    api::Reader in(inHandle, false);

    api::Frame frame;

    std::vector<std::vector<double>> data;
    std::vector<api::StridedData> strides;

    std::vector<std::string> readColumnNames;
    std::vector<api::StridedData> readData;

    while ((frame = in.next())) {
      const std::vector<api::ColumnInfo> &readColumns = frame.columnInfo();
      const size_t rowCount = frame.rowCount();

      readColumnNames.clear();
      for (const api::ColumnInfo &column : readColumns)
        readColumnNames.push_back(column.name);
      for (const std::string &name : readColumnNames)
        if (numValueChangesInColumn.find(name) == numValueChangesInColumn.end())
          columnNamesInOriginalOrder.push_back(name);

      if (data.size() < readColumns.size())
        data.resize(readColumns.size());
      readData.clear();
      for (size_t col = 0; col < readColumns.size(); ++col) {
        data[col].resize(rowCount);
        readData.emplace_back(data[col].data(), rowCount, sizeof(double), sizeof(double));
      }

      api::Decoder decoder(readColumnNames, readData);
      decoder.decode(frame);

      for (size_t columnIndex = 0; columnIndex < readColumns.size(); ++columnIndex) {
        // Count the number of times values in the current column change from row to row
        size_t numChanges = 0;
        const std::vector<double> &columnData = data[columnIndex];
        for (size_t row = 1; row < columnData.size(); ++row)
          if (columnData[row] != columnData[row - 1])
            ++numChanges;
        numValueChangesInColumn[readColumnNames[columnIndex]] += numChanges;
      }
    }
  }

  std::cout << "Number of value changes in each column:\n";
  for (const std::string &name : columnNamesInOriginalOrder)
    std::cout << "  " << name << ": " << numValueChangesInColumn.at(name) << "\n";
  std::cout << std::endl;

  struct ColumnStats {
    std::string name;
    size_t numValueChanges;
  };

  std::vector<ColumnStats> stats;
  for (const std::string &name : columnNamesInOriginalOrder)
    stats.push_back({name, numValueChangesInColumn.at(name)});

  std::stable_sort(stats.begin(), stats.end(),
                   [](const ColumnStats &a, const ColumnStats &b) {
                     return a.numValueChanges < b.numValueChanges;
                   });

  std::vector<std::string> optimalColumnOrder;
  for (const ColumnStats &stat : stats)
    optimalColumnOrder.push_back(stat.name);

  return optimalColumnOrder;
}

void printOptimalColumnOrder(const std::vector<std::string> &columns) {
  std::cout << "Optimal column order:\n";
  for (const std::string &column : columns)
    std::cout << "  " << column << "\n";
  std::cout << std::endl;
}

void reorderColumns(const PathName &inFile, const PathName &outFile,
                    const std::vector<std::string> &columnOrder) {
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
    for (const std::string &columnName : columnOrder) {
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

} // namespace

OrderColumnsTool::OrderColumnsTool (int argc, char *argv[]) : Tool(argc, argv) { }

void OrderColumnsTool::run() {
  if (parameters().size() != 2 && parameters().size() != 3)
  {
    Log::error() << "Usage: ";
    usage(parameters(0), Log::error());
    Log::error() << std::endl;
    throw UserError("Expected 2 or 3 command line parameters");
  }

  api::Settings::treatIntegersAsDoubles(false);
  api::Settings::setDoubleMissingValue(2147483647.0);

  PathName inFile = parameters(1);

  // Find the column order that will minimise the number of values that need to be encoded
  // (by moving the most frequently varying columns to the end).
  const std::vector<std::string> optimalColumnOrder = findOptimalColumnOrder(inFile);
  printOptimalColumnOrder(optimalColumnOrder);
  // Read data from the input file and write them to the output file in the optimum column order.
  if (parameters().size() == 3) {
    PathName outFile = parameters(2);
    reorderColumns(inFile, outFile, optimalColumnOrder);
  }
}

void OrderColumnsTool::help(std::ostream &o) {
  o << "Determines column order likely to minimize file size and read time.";
}

void OrderColumnsTool::usage(const std::string& name, std::ostream &o) {
  o << name << " <input.odb> [<output.odb>]\n\n"
       "  <input.odb>:  Name of the input file.\n"
       "  <output.odb>: Name of the output file. Optional; if provided, the tool\n"
       "                will not only determine and print the optimal column order\n"
       "                for the data loaded from the input file, but also write an\n"
       "                output file containing the same data as the input file,\n"
       "                but with columns rearranged in that order.";
}


} // namespace tool 
} // namespace odc 

