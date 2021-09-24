/*
 * (C) Copyright 2021 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#ifndef OrderColumnsTool_H
#define OrderColumnsTool_H

#include "odc/tools/Tool.h"

#include <ostream>
#include <string>

namespace odc {
namespace tool {

class OrderColumnsTool : public Tool {
public:
  OrderColumnsTool (int argc, char *argv[]);

  OrderColumnsTool(const OrderColumnsTool &) = delete;
  OrderColumnsTool &operator=(const OrderColumnsTool &) = delete;

  void run();

  static void help(std::ostream &o);

  static void usage(const std::string& name, std::ostream &o);
};

template <> struct ExperimentalTool<OrderColumnsTool> { enum { experimental = false }; };

} // namespace tool 
} // namespace odc 

#endif 
