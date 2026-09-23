/*****************************************************************************
 * Copyright (c) 2014-2026 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "RideTypes.h"

#include <memory>
#include <string_view>

struct ITrackImporter;

inline constexpr std::string_view kBuiltinEuroFighterTrackDesignPath = "builtin://Euro-Fighter.td7";
inline constexpr ride_type_t kBuiltinEuroFighterRideType = 44; // Vertical Drop Roller Coaster

bool IsBuiltinTrackDesignPath(std::string_view path);
std::unique_ptr<ITrackImporter> CreateBuiltinTrackDesignImporter(std::string_view path);
