/*****************************************************************************
 * Copyright (c) 2014-2026 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include <string_view>

struct ScenarioIndexEntry;

namespace OpenRCT2::Scenario
{
    inline constexpr std::string_view kBuiltinSixFlagsNewEnglandPath = "builtin://Six Flags New England.park";

    bool IsBuiltinSixFlagsNewEnglandPath(std::string_view path);
    ::ScenarioIndexEntry CreateBuiltinSixFlagsNewEnglandIndexEntry();
    bool LoadBuiltinSixFlagsNewEnglandScenario();
} // namespace OpenRCT2::Scenario
