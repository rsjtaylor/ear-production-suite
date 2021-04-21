#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "pluginsuite.h"
#include "projectelements.h"

namespace admplug {
    class Track;
    class PluginParameter;

    class EARPluginSuite : public PluginSuite
    {

    public:
        EARPluginSuite();
        ~EARPluginSuite();

        void onProjectBuildBegin(std::shared_ptr<IADMMetaData> metadata,
                                 std::shared_ptr<ImportListener> broadcaster,
                                 const ReaperAPI &api) override;
        void onCreateProject(const ProjectNode &rootNode, const ReaperAPI &api) override;
        void onCreateObjectTrack(const TrackElement &, const ReaperAPI &api) override;
        void onCreateDirectTrack(const TrackElement &, const ReaperAPI &api) override;
        void onCreateHoaTrack(const TrackElement &, const ReaperAPI &api) override;
        void onCreateGroup(const TrackElement &, const ReaperAPI &api) override;
        void onObjectAutomation(const ObjectAutomation&, const ReaperAPI &api) override;
        void onDirectSpeakersAutomation(const DirectSpeakersAutomation&, const ReaperAPI &api) override;
        void onHoaAutomation(const HoaAutomation&, const ReaperAPI &api) override;
        void onProjectBuildComplete(const ReaperAPI &api) override;
        bool pluginSuiteUsable(const ReaperAPI &api) override;
        bool representAdmStructureWithGroups(ReaperAPI const& api) override;
        bool applyFXPreset(const HoaAutomation &, const ReaperAPI &api) override;
        PluginParameter* getPluginParameterFor(AdmParameter admParameter) override;
        Parameter* getParameterFor(AdmParameter admParameter) override;
        std::vector<ADMChannel> reorderAndFilter(std::vector<ADMChannel> const& channels, ReaperAPI const& api) override;

        static const char* OBJECT_METADATA_PLUGIN_NAME;
        static const char* DIRECTSPEAKERS_METADATA_PLUGIN_NAME;
        static const char* SCENEMASTER_PLUGIN_NAME;
        static const char* RENDERER_PLUGIN_NAME;

        static int countChannelsInSpeakerLayout(int slIndex);

    private:
        std::vector<std::unique_ptr<PluginParameter>> const & automatedObjectPluginParameters();
        std::vector<std::unique_ptr<TrackParameter>> const & trackParameters();
        std::shared_ptr<PluginParameter> objectTrackMappingParameter;
        std::shared_ptr<PluginParameter> directSpeakersTrackMappingParameter;
        std::shared_ptr<PluginParameter> directSpeakersLayoutParameter;
        std::shared_ptr<ImportListener> broadcaster;
        std::unique_ptr<UniqueValueAssigner> trackMappingAssigner;

        std::unique_ptr<Track> createBusTrack(std::string pluginName, const ReaperAPI &api);
        std::unique_ptr<Track> sceneMasterTrack;
        std::unique_ptr<Track> rendererTrack;
        void checkForExistingTracks(const ReaperAPI &api);
        int trackInsertionIndex{ -1 };
        void setTrackInsertionIndexFromSelectedMedia(const ReaperAPI &api);

        std::string originalAdmDocument;
        std::vector<uint32_t> trackMappingToAtu; // Index = trackMapping Parameter, value = AudioTrackUid
        bool sceneMasterAlreadyExisted{ false };

        static const int MAX_CHANNEL_COUNT;

        static bool registered;
        
        void reportClippedParameters();
        
        std::unordered_map<AdmParameter, std::pair<int, const std::string>> clippedParamInfo {
          {AdmParameter::OBJECT_AZIMUTH,                    {0, {"Azimuth"}}},
          {AdmParameter::OBJECT_ELEVATION,                  {0, {"Elevation"}}},
          {AdmParameter::OBJECT_DISTANCE,                   {0, {"Distance"}}},
          {AdmParameter::OBJECT_X,                          {0, {"X"}}},
          {AdmParameter::OBJECT_Y,                          {0, {"Y"}}},
          {AdmParameter::OBJECT_Z,                          {0, {"Z"}}},
          {AdmParameter::OBJECT_HEIGHT,                     {0, {"Height"}}},
          {AdmParameter::OBJECT_WIDTH,                      {0, {"Width"}}},
          {AdmParameter::OBJECT_DEPTH,                      {0, {"Depth"}}},
          {AdmParameter::OBJECT_GAIN,                       {0, {"Gain"}}},
          {AdmParameter::OBJECT_DIFFUSE,                    {0, {"Diffuse"}}},
          {AdmParameter::OBJECT_DIVERGENCE,                 {0, {"Divergence"}}},
          {AdmParameter::OBJECT_DIVERGENCE_AZIMUTH_RANGE,   {0, {"Divergence Azimuth Range"}}},
          {AdmParameter::SPEAKER_AZIMUTH,                   {0, {"Speaker Azimuth"}}},
          {AdmParameter::SPEAKER_ELEVATION,                 {0, {"Speaker Elevation"}}},
          {AdmParameter::SPEAKER_DISTANCE,                  {0, {"Speaker Distance"}}},
          {AdmParameter::NFC_REF_DIST,                      {0, {"NFC_REF_DIST"}}},
          {AdmParameter::NONE,                              {0, {"None"}}},
        };

    };
}
