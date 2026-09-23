/*****************************************************************************
 * Copyright (c) 2014-2026 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "BuiltinTrackDesigns.h"

#include "../TrackImporter.h"
#include "../drawing/Colour.h"
#include "Ride.h"
#include "TrackDesign.h"

#include <string>

using namespace OpenRCT2;
using namespace OpenRCT2::Drawing;

namespace
{
    TrackDesignTrackElement MakeTrackElement(TrackElemType type, bool hasChain = false, uint8_t brakeSpeed = 0)
    {
        TrackDesignTrackElement element{};
        element.type = type;
        element.brakeBoosterSpeed = brakeSpeed;
        if (hasChain)
        {
            element.flags.set(TrackDesignTrackElementFlag::hasChain);
        }
        return element;
    }

    std::unique_ptr<TrackDesign> CreateEuroFighterTrackDesign()
    {
        auto td = std::make_unique<TrackDesign>();

        // OpenRCT2's Vertical Drop Roller Coaster is the closest native track/vehicle set to a
        // Gerstlauer Euro-Fighter. The design deliberately uses only pieces supported by that ride type.
        td->trackAndVehicle.rtdIndex = kBuiltinEuroFighterRideType;
        td->trackAndVehicle.vehicleObject = ObjectEntryDescriptor("rct2.ride.bmvd");
        td->trackAndVehicle.numberOfTrains = 1;
        td->trackAndVehicle.numberOfCarsPerTrain = 3;

        td->operation.rideMode = RideMode::continuousCircuit;
        td->operation.liftHillSpeed = 6;
        td->operation.numCircuits = 1;
        td->operation.minWaitingTime = 10;
        td->operation.maxWaitingTime = 60;

        td->appearance.vehicleColourSettings = VehicleColourSettings::same;
        for (auto& colours : td->appearance.trackColours)
        {
            colours = { Colour::saturatedRed, Colour::brightRed, Colour::black };
        }
        for (auto& colours : td->appearance.vehicleColours)
        {
            colours = { Colour::black, Colour::brightRed, Colour::yellow };
        }

        // Side 1: three-tile station and run-out.
        td->trackElements.push_back(MakeTrackElement(TrackElemType::beginStation));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::middleStation));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::endStation));
        for (int32_t i = 0; i < 7; i++)
        {
            td->trackElements.push_back(MakeTrackElement(TrackElemType::flat));
        }
        td->trackElements.push_back(MakeTrackElement(TrackElemType::rightQuarterTurn3Tiles));

        // Side 2: signature near-vertical/vertical chain lift. Vertical chain is valid when a
        // design is placed even though it is not normally exposed by every construction UI path.
        td->trackElements.push_back(MakeTrackElement(TrackElemType::flat));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::flat));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::flatToUp60, true));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::up60ToUp90, true));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::up90, true));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::up90, true));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::up90, true));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::up90ToUp60, true));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::up60ToFlat, true));
        for (int32_t i = 0; i < 5; i++)
        {
            td->trackElements.push_back(MakeTrackElement(TrackElemType::flat));
        }
        td->trackElements.push_back(MakeTrackElement(TrackElemType::rightQuarterTurn3Tiles));

        // Side 3: two-tile hold at the top followed by the 90-degree signature drop.
        td->trackElements.push_back(MakeTrackElement(TrackElemType::flat));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::flat));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::flatToDown60));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::down60ToDown90));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::down90));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::down90));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::down90));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::down90ToDown60));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::down60ToFlat));
        for (int32_t i = 0; i < 5; i++)
        {
            td->trackElements.push_back(MakeTrackElement(TrackElemType::flat));
        }
        td->trackElements.push_back(MakeTrackElement(TrackElemType::rightQuarterTurn3Tiles));

        // Side 4: compact double-loop inversion section, then final brakes into the station.
        td->trackElements.push_back(MakeTrackElement(TrackElemType::flat));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::flat));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::flat));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::leftVerticalLoop));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::rightVerticalLoop));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::flat));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::brakes, false, 4));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::brakes, false, 4));
        td->trackElements.push_back(MakeTrackElement(TrackElemType::rightQuarterTurn3Tiles));

        td->statistics.inversions = 2;
        td->statistics.drops = 1;
        td->statistics.highestDropHeight = 32;
        td->statistics.spaceRequired = { 16, 16 };
        td->version = RCT12::TD46Version::td7;
        return td;
    }

    class BuiltinTrackDesignImporter final : public ITrackImporter
    {
    private:
        std::string _path;

    public:
        explicit BuiltinTrackDesignImporter(std::string_view path)
            : _path(path)
        {
        }

        bool Load(const utf8* path) override
        {
            _path = path == nullptr ? std::string{} : std::string(path);
            return IsBuiltinTrackDesignPath(_path);
        }

        bool LoadFromStream(IStream*) override
        {
            return false;
        }

        std::unique_ptr<TrackDesign> Import() override
        {
            if (_path == kBuiltinEuroFighterTrackDesignPath)
            {
                return CreateEuroFighterTrackDesign();
            }
            return nullptr;
        }
    };
} // namespace

bool IsBuiltinTrackDesignPath(std::string_view path)
{
    return path == kBuiltinEuroFighterTrackDesignPath;
}

std::unique_ptr<ITrackImporter> CreateBuiltinTrackDesignImporter(std::string_view path)
{
    return std::make_unique<BuiltinTrackDesignImporter>(path);
}
