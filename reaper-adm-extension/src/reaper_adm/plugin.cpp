#include <stdexcept>
#include <cassert>
#include <cmath>
#include "plugin.h"
#include "reaperapi.h"
#include "parameter.h"
#include "envelopecreator.h"
#include "automationenvelope.h"

using namespace admplug;

PluginInstance::PluginInstance(MediaTrack* mediaTrack, std::string pluginName, bool shouldInsert, ReaperAPI const& api) : track{mediaTrack, api}, name{pluginName}, api{api}
{
    auto index = api.TrackFX_AddByName(mediaTrack, pluginName.c_str(), false, shouldInsert? TrackFXAddMode::CreateNew : TrackFXAddMode::QueryPresence);
    if(index < 0) {
        throw std::runtime_error("Could not add to or get plugin from track");
    }
    guid = std::make_unique<ReaperGUID>(api.TrackFX_GetFXGUID(mediaTrack, index));
}

admplug::PluginInstance::PluginInstance(MediaTrack * mediaTrack, int pluginIndex, const ReaperAPI & api) : track{mediaTrack, api}, api{api}
{
    char cName[100];
    if(!api.TrackFX_GetFXName(mediaTrack, pluginIndex, cName, 100)) {
        throw std::runtime_error("Plugin index appears to be invalid");
    }
    name = cName;
    guid = std::make_unique<ReaperGUID>(api.TrackFX_GetFXGUID(mediaTrack, pluginIndex));
}

admplug::PluginInstance::PluginInstance(MediaTrack* mediaTrack, const ReaperAPI & api) : track{ mediaTrack, api }, api{ api }
{
    // To be called only by child classes.
    // Child classes MUST set the following members in their constructor;
    //   guid
    //   name
}

std::unique_ptr<AutomationEnvelope> PluginInstance::getEnvelope(const PluginParameter &parameter, EnvelopeCreator const& creator) const
{
    auto trackEnvelope = api.getPluginEnvelope(track.get(), name.c_str(), parameter.index());
    return creator.create(trackEnvelope, api);
}

void PluginInstance::setParameter(const PluginParameter &parameter,
                                  ParameterValue value) const
{
    auto index = getPluginIndex();
    if(!(index < 0)) {
      api.TrackFX_SetParam(track.get(), index, parameter.index(), value.get());
    }
}

void admplug::PluginInstance::setParameterWithConvert(PluginParameter const & parameter, ParameterValue value) const
{
    setParameter(parameter, parameter.forwardMap(value));
}

std::optional<ParameterValue>
PluginInstance::getParameter(const PluginParameter &parameter) const {
    auto index = getPluginIndex();
    if(index < 0) {
        return {};
    }
    auto value = ParameterValue{api.TrackFX_GetParamNormalized(track.get(), index, parameter.index())};
    if(value.get() < 0) {
        return {};
    }
    return value;
}

std::optional<ParameterValue> admplug::PluginInstance::getParameterWithConvert(const PluginParameter & parameter) const
{
    auto raw = getParameter(parameter);
    if(!raw.has_value()) return {};
    return std::optional<ParameterValue>(parameter.reverseMap(ParameterValue{*raw}));
}

std::optional<int> admplug::PluginInstance::getParameterWithConvertToInt(const PluginParameter & parameter) const
{
    auto optVal = getParameterWithConvert(parameter);
    if(!optVal.has_value()) return {};
    int val = (int)std::round(optVal->get());
    return std::optional<int>(val);
}

int admplug::PluginInstance::getPluginIndex() const
{
    assert(guid);
    int numFx = api.TrackFX_GetCount(track.get());
    for(int fxNum = 0; fxNum < numFx; fxNum++) {
        auto fxGuid = api.TrackFX_GetFXGUID(track.get(), fxNum);
        if(ReaperGUID(fxGuid) == *guid) {
            return fxNum;
        }
    }
    return -1;
}

std::optional<std::string> admplug::PluginInstance::getPluginName()
{
    char cName[100];
    if(api.TrackFX_GetFXName(getTrackInstance().get(), getPluginIndex(), cName, 100))
        return std::optional<std::string>(cName);
    return std::optional<std::string>();
}

bool admplug::PluginInstance::stillExists() const
{
    return getPluginIndex() >= 0;
}

TrackInstance& admplug::PluginInstance::getTrackInstance()
{
    return track;
}

bool admplug::PluginInstance::isPluginBypassed() const
{
    auto index = getPluginIndex();
    assert(index >= 0);
    return !api.TrackFX_GetEnabled(track.get(), index);
}

bool admplug::PluginInstance::isPluginOffline() const
{
    auto index = getPluginIndex();
    assert(index >= 0);
    return api.TrackFX_GetOffline(track.get(), index);
}

bool admplug::PluginInstance::isBypassed() const
{
    return isPluginBypassed() || track.isPluginChainBypassed();
}
