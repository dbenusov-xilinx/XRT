// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2024 Advanced Micro Devices, Inc. - All rights reserved

#include "ReportAie2Mem.h"

#include "Aie2Utilities.h"
#include "core/common/info_aie.h"
#include "tools/common/Table2D.h"
#include "tools/common/XBUtilities.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#define fmt8(x) boost::format("%8s%-22s: " x "\n") % " "

void
ReportAie2Mem::
getPropertyTreeInternal(const xrt_core::device* dev,
                        boost::property_tree::ptree& pt) const
{
  // Defer to the 20202 format.  If we ever need to update JSON data, 
  // Then update this method to do so.
  getPropertyTree20202(dev, pt);
}

void 
ReportAie2Mem::
getPropertyTree20202(const xrt_core::device* dev,
                     boost::property_tree::ptree& pt) const
{
  pt.add_child("aie_mem", asd_parser::get_formated_tiles_info(dev, asd_parser::aie_tile_type::mem));
}

void
ReportAie2Mem::
writeReport(const xrt_core::device* /*dev*/,
            const boost::property_tree::ptree& pt,
            const std::vector<std::string>& /*filter*/,
            std::ostream& output) const
{
  boost::property_tree::ptree empty_ptree;
  std::vector<std::string> aieCoreList;

  output << "AIE Mem Tiles\n";

  if (!pt.get_child_optional("aie_mem.columns")) {
    output << "  No AIE columns are active on the device\n\n";
    return;
  }

  for (const auto& [column_name, column] : pt.get_child("aie_mem.columns")) {

    output << boost::format("  Column %s\n") % column.get<std::string>("col");

    const auto column_status = column.get<std::string>("status");
    output << boost::format("    Status: %s\n") % column_status;

    if (boost::iequals(column_status, "inactive"))
      continue;

    output << "    Tiles\n";
    for (const auto& [tile_name, tile] : column.get_child("tiles")) {
      output << boost::format("      Row %d\n") % tile.get<int>("row");

      output << "        DMA MM2S Channels:\n";
      Table2D mm2s_table = generate_channel_table(tile.get_child("dma.mm2s_channels"));
      output << mm2s_table.toString("          ");

      output << "        DMA S2MM Channels:\n";
      Table2D s2mm_table = generate_channel_table(tile.get_child("dma.s2mm_channels"));
      output << s2mm_table.toString("          ");

      output << "        Locks:\n";
      for (const auto& [lock_name, lock] : tile.get_child("locks")) {
        output << boost::format("          %s\n") % lock.get_value<std::string>();
      }
      output << std::endl;
    }
  }
}
