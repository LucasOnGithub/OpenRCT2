/*****************************************************************************
 * Copyright (c) 2014-2026 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "BuiltinSixFlagsNewEngland.h"

#include "../Context.h"
#include "../Diagnostic.h"
#include "../Game.h"
#include "../GameState.h"
#include "../OpenRCT2.h"
#include "../actions/CommandFlag.h"
#include "../actions/GameActionRunner.h"
#include "../actions/ride/RideSetNameAction.h"
#include "../actions/track/TrackDesignAction.h"
#include "../localisation/LocalisationService.h"
#include "../object/DefaultObjects.h"
#include "../object/ObjectManager.h"
#include "../object/ObjectRepository.h"
#include "../ride/Ride.h"
#include "../ride/TrackDesign.h"
#include "../ride/ted/TrackElemType.h"
#include "../world/Map.h"
#include "../world/MapLimits.h"
#include "../world/Park.h"
#include "../world/map_generator/MapGen.h"
#include "../world/tile_element/SurfaceElement.h"
#include "Scenario.h"
#include "ScenarioRepository.h"

#include <array>
#include <string>
#include <string_view>

using OpenRCT2::GameActions::CommandFlag;

namespace OpenRCT2::Scenario
{
    namespace
    {
        constexpr ride_type_t kRideTypeInverted = 3;
        constexpr ride_type_t kRideTypeJunior = 4;
        constexpr ride_type_t kRideTypeTwister = 51;
        constexpr ride_type_t kRideTypeWooden = 52;
        constexpr ride_type_t kRideTypeSteelWildMouse = 54;
        constexpr ride_type_t kRideTypeCompactInverted = 73;
        constexpr ride_type_t kRideTypeLimLaunched = 90;
        constexpr ride_type_t kRideTypeHypercoaster = 91;
        constexpr ride_type_t kRideTypeSpinningWildMouse = 94;

        struct CoasterSpec
        {
            std::string_view name;
            ride_type_t rideType;
            std::string_view vehicleObject;
            TileCoordsXY location;
            bool launched{};
            bool tightTurns{};
            uint8_t carsPerTrain{ 5 };
        };

        constexpr std::array<CoasterSpec, 12> kCoasters = {
            CoasterSpec{ "Quantum Accelerator", kRideTypeLimLaunched, "rct2.ride.premt1", { 18, 18 }, true, false, 5 },
            CoasterSpec{ "SUPERMAN The Ride", kRideTypeHypercoaster, "rct2.ride.arrt2", { 54, 18 }, false, false, 6 },
            CoasterSpec{ "Wicked Cyclone", kRideTypeWooden, "rct2.ride.ptct1", { 90, 18 }, false, false, 5 },
            CoasterSpec{ "BATMAN THE DARK KNIGHT", kRideTypeTwister, "rct2.ride.bmfl", { 126, 18 }, false, false, 5 },
            CoasterSpec{ "THE RIDDLER Revenge", kRideTypeInverted, "rct2.ride.nemt", { 18, 60 }, false, false, 5 },
            CoasterSpec{ "THE JOKER 4D Free Fly Coaster", kRideTypeCompactInverted, "rct2.ride.slct", { 54, 60 }, false, false, 5 },
            CoasterSpec{ "Pandemonium", kRideTypeSpinningWildMouse, "rct2.ride.wmspin", { 90, 60 }, false, true, 1 },
            CoasterSpec{ "GOTHAM CITY GAUNTLET ESCAPE FROM ARKHAM ASYLUM", kRideTypeSteelWildMouse, "rct2.ride.smc1", { 126, 60 }, false, true, 1 },
            CoasterSpec{ "Thunderbolt", kRideTypeWooden, "rct2.ride.ptct1", { 18, 102 }, false, false, 5 },
            CoasterSpec{ "Flashback", kRideTypeLimLaunched, "rct2.ride.premt1", { 54, 102 }, true, false, 5 },
            CoasterSpec{ "CATWOMAN Whip", kRideTypeJunior, "rct2.ride.zldb", { 90, 102 }, false, false, 5 },
            CoasterSpec{ "The Great Chase", kRideTypeJunior, "rct2.ride.zldb", { 126, 102 }, false, false, 5 },
        };

        TrackDesignTrackElement MakeTrackElement(TrackElemType type, bool chain = false, uint8_t speed = 0)
        {
            TrackDesignTrackElement element{};
            element.type = type;
            element.brakeBoosterSpeed = speed;
            if (chain)
            {
                element.flags.set(TrackDesignTrackElementFlag::hasChain);
            }
            return element;
        }

        void AddFlat(TrackDesign& td, int32_t count)
        {
            for (int32_t i = 0; i < count; i++)
            {
                td.trackElements.push_back(MakeTrackElement(TrackElemType::flat));
            }
        }

        TrackDesign CreateCoasterDesign(const CoasterSpec& spec)
        {
            TrackDesign td{};
            td.trackAndVehicle.rtdIndex = spec.rideType;
            td.trackAndVehicle.vehicleObject = ObjectEntryDescriptor(spec.vehicleObject);
            td.trackAndVehicle.numberOfTrains = spec.tightTurns ? 4 : 2;
            td.trackAndVehicle.numberOfCarsPerTrain = spec.carsPerTrain;

            td.operation.rideMode = RideMode::continuousCircuit;
            td.operation.liftHillSpeed = 6;
            td.operation.numCircuits = 1;
            td.operation.minWaitingTime = 10;
            td.operation.maxWaitingTime = 60;

            td.appearance.vehicleColourSettings = VehicleColourSettings::same;

            const auto quarterTurn = spec.tightTurns ? TrackElemType::rightQuarterTurn1Tile
                                                     : TrackElemType::rightQuarterTurn3Tiles;

            // Side one: station and launch/run-out.
            td.trackElements.push_back(MakeTrackElement(TrackElemType::beginStation));
            td.trackElements.push_back(MakeTrackElement(TrackElemType::middleStation));
            td.trackElements.push_back(MakeTrackElement(TrackElemType::endStation));
            if (spec.launched)
            {
                td.trackElements.push_back(MakeTrackElement(TrackElemType::booster, false, 20));
                td.trackElements.push_back(MakeTrackElement(TrackElemType::booster, false, 20));
                AddFlat(td, 3);
            }
            else
            {
                AddFlat(td, 5);
            }
            td.trackElements.push_back(MakeTrackElement(quarterTurn));

            // Side two: lift hill (or second launch for Quantum/Flashback).
            if (spec.launched)
            {
                td.trackElements.push_back(MakeTrackElement(TrackElemType::booster, false, 20));
                td.trackElements.push_back(MakeTrackElement(TrackElemType::booster, false, 20));
                AddFlat(td, 6);
            }
            else
            {
                td.trackElements.push_back(MakeTrackElement(TrackElemType::flatToUp25, true));
                for (int32_t i = 0; i < 4; i++)
                {
                    td.trackElements.push_back(MakeTrackElement(TrackElemType::up25, true));
                }
                td.trackElements.push_back(MakeTrackElement(TrackElemType::up25ToFlat, true));
                AddFlat(td, 2);
            }
            td.trackElements.push_back(MakeTrackElement(quarterTurn));

            // Side three returns to station height.
            if (spec.launched)
            {
                AddFlat(td, 8);
            }
            else
            {
                AddFlat(td, 2);
                td.trackElements.push_back(MakeTrackElement(TrackElemType::flatToDown25));
                for (int32_t i = 0; i < 4; i++)
                {
                    td.trackElements.push_back(MakeTrackElement(TrackElemType::down25));
                }
                td.trackElements.push_back(MakeTrackElement(TrackElemType::down25ToFlat));
            }
            td.trackElements.push_back(MakeTrackElement(quarterTurn));

            // Side four and final turn close the circuit.
            AddFlat(td, 8);
            td.trackElements.push_back(MakeTrackElement(quarterTurn));

            td.version = RCT12::TD46Version::td7;
            return td;
        }

        void LoadScenarioObjects()
        {
            auto* context = GetContext();
            auto& objectRepository = context->GetObjectRepository();
            auto& objectManager = context->GetObjectManager();
            objectRepository.LoadOrConstruct(context->GetLocalisationService().GetCurrentLanguage());

            objectManager.UnloadAll();

            for (const auto& id : kMinimumRequiredObjects)
            {
                objectManager.LoadObject(id);
            }
            for (const auto& id : kCommonScenarioAndTrackDesignerObjects)
            {
                objectManager.LoadObject(id);
            }
            for (const auto& id : kDefaultScenarioObjects)
            {
                objectManager.LoadObject(id);
            }

            constexpr std::array<std::string_view, 8> extraRideObjects = {
                "rct2.ride.premt1",
                "rct2.ride.arrt2",
                "rct2.ride.bmfl",
                "rct2.ride.nemt",
                "rct2.ride.slct",
                "rct2.ride.wmspin",
                "rct2.ride.smc1",
                "rct2.ride.bmvd",
            };
            for (const auto& id : extraRideObjects)
            {
                objectManager.LoadObject(id);
            }
        }

        void OwnEntirePark(GameState_t& gameState)
        {
            OwnershipFlags owned{};
            owned.set(OwnershipFlag::landOwned);

            for (int32_t y = 1; y < gameState.mapSize.y - 1; y++)
            {
                for (int32_t x = 1; x < gameState.mapSize.x - 1; x++)
                {
                    if (auto* surface = MapGetSurfaceElementAt(TileCoordsXY{ x, y }); surface != nullptr)
                    {
                        surface->setOwnership(owned);
                    }
                }
            }
        }

        void PlaceCoasters(GameState_t& gameState)
        {
            const int32_t baseZ = 14 * kCoordsZStep;

            for (const auto& spec : kCoasters)
            {
                auto td = CreateCoasterDesign(spec);
                CoordsXYZD location{
                    spec.location.x * kCoordsXYStep,
                    spec.location.y * kCoordsXYStep,
                    baseZ,
                    0,
                };

                GameActions::TrackDesignAction placeAction(location, td, false, RideInspection::every30Minutes);
                placeAction.SetFlags({ CommandFlag::noSpend, CommandFlag::allowDuringPaused });
                auto result = GameActions::Execute(&placeAction, gameState);
                if (result.error != GameActions::Status::ok)
                {
                    LOG_ERROR("SFNE: failed to place %s", spec.name.data());
                    continue;
                }

                const auto rideId = result.getData<RideId>();
                GameActions::RideSetNameAction nameAction(rideId, std::string(spec.name));
                nameAction.SetFlags({ CommandFlag::noSpend, CommandFlag::allowDuringPaused });
                GameActions::Execute(&nameAction, gameState);
            }
        }
    } // namespace

    bool IsBuiltinSixFlagsNewEnglandPath(std::string_view path)
    {
        return path == kBuiltinSixFlagsNewEnglandPath;
    }

    ScenarioIndexEntry CreateBuiltinSixFlagsNewEnglandIndexEntry()
    {
        ScenarioIndexEntry entry{};
        entry.Path = std::string(kBuiltinSixFlagsNewEnglandPath);
        entry.Timestamp = 0;
        entry.Category = Category::real;
        entry.SourceGame = ScenarioSource::real;
        entry.SourceIndex = -1;
        entry.ScenarioId = SC_UNIDENTIFIED;
        entry.ObjectiveType = ObjectiveType::none;
        entry.InternalName = "Six Flags New England";
        entry.Name = "Six Flags New England";
        entry.Details = "A 2026 recreation of Six Flags New England, including Quantum Accelerator and the park's current coaster lineup.";
        return entry;
    }

    bool LoadBuiltinSixFlagsNewEnglandScenario()
    {
        auto& gameState = getGameState();

        gameStateInitAll(gameState, { 180, 150 });
        LoadScenarioObjects();
        ContextResetSubsystems();

        World::MapGenerator::Settings mapSettings{};
        mapSettings.algorithm = World::MapGenerator::Algorithm::blank;
        mapSettings.mapSize = { 180, 150 };
        mapSettings.waterLevel = 6;
        mapSettings.heightmapLow = 14;
        mapSettings.trees = false;
        mapSettings.beaches = false;
        World::MapGenerator::generate(&mapSettings);

        OwnEntirePark(gameState);

        auto& objectManager = GetContext()->GetObjectManager();
        gameState.lastEntranceStyle = objectManager.GetLoadedObjectEntryIndex("rct2.station.plain");

        gameState.scenarioOptions.category = Category::real;
        gameState.scenarioOptions.name = "Six Flags New England";
        gameState.scenarioOptions.details =
            "A 2026 recreation of Six Flags New England, including Quantum Accelerator and the current attraction lineup.";
        gameState.scenarioOptions.objective.Type = ObjectiveType::none;
        gameState.scenarioOptions.initialCash = 100000.00_GBP;

        gameState.park.name = "Six Flags New England";
        gameState.park.flags.set(ParkFlag::noMoney);
        gameState.park.flags.set(ParkFlag::freeEntry);
        gameState.park.flags.set(ParkFlag::parkOpen);
        gameState.park.flags.set(ParkFlag::showRealGuestNames);

        gameState.cheats.ignoreResearchStatus = true;
        gameState.cheats.sandboxMode = true;
        gameState.cheats.buildInPauseMode = true;

        PlaceCoasters(gameState);

        gameState.scenarioFileName = "Six Flags New England.park";
        gScenarioSavePath = std::string(kBuiltinSixFlagsNewEnglandPath);
        gCurrentLoadedPath = std::string(kBuiltinSixFlagsNewEnglandPath);
        gFirstTimeSaving = true;
        gLegacyScene = LegacyScene::playing;

        GameFixSaveVars();
        ScenarioBegin(gameState);
        GameLoadScripts();
        GameNotifyMapChanged();
        return true;
    }
} // namespace OpenRCT2::Scenario
