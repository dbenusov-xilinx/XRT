// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2024 Advanced Micro Devices, Inc. - All rights reserved

#ifndef __ReportAie2Utilities_h_
#define __ReportAie2Utilities_h_

#include "core/common/info_aie.h"
#include "tools/common/Table2D.h"

#include <boost/property_tree/ptree.hpp>

static const std::vector<Table2D::HeaderData> dma_table_headers = {
  {"Status", Table2D::Justification::left},
  {"Queue Size", Table2D::Justification::left},
  {"Queue Status", Table2D::Justification::left},
  {"Current BD", Table2D::Justification::left}
};

static Table2D
generate_channel_table(const boost::property_tree::ptree& channels)
{
    Table2D table(dma_table_headers);
    for (const auto& [node_name, node] : channels) {
      const std::vector<std::string> entry_data = {
        node.get<std::string>("status"),
        node.get<std::string>("queue_size") ,
        node.get<std::string>("queue_status"),
        node.get<std::string>("current_bd")
      };
      table.addEntry(entry_data);
    }
    return table;
}

#endif
