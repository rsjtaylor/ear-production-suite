#include <cmath>
#include <algorithm>
#include "parametervaluemapping.h"

using namespace admplug;


AutomationPoint admplug::ParameterValueMapping::operator()(AutomationPoint point) const
{
    return forwardMap(point);
}

AutomationPoint admplug::ParameterValueMapping::forwardMap(AutomationPoint point) const
{
    return AutomationPoint(point.time(), point.duration(), forwardMap(point.value()));
}

AutomationPoint admplug::ParameterValueMapping::reverseMap(AutomationPoint point) const
{
    return AutomationPoint(point.time(), point.duration(), reverseMap(point.value()));
}

FunctionalMapping::FunctionalMapping(
    std::function<ParameterValue(ParameterValue)> forwardMapper,
    std::function<ParameterValue(ParameterValue)> reverseMapper)
    : forwardMapper{forwardMapper}, reverseMapper{reverseMapper} {}

ParameterValue admplug::FunctionalMapping::forwardMap(ParameterValue val) const
{
    return forwardMapper(val);
}

ParameterValue admplug::FunctionalMapping::reverseMap(ParameterValue val) const
{
    return reverseMapper(val);
}

CompositeMapping::CompositeMapping(std::vector<std::shared_ptr<const ParameterValueMapping> > mappings) : mappings{mappings}
{
}

ParameterValue admplug::CompositeMapping::forwardMap(ParameterValue val) const
{
    for(int i = 0; i < mappings.size(); i++) {
        val = mappings[i]->forwardMap(val);
    }
    return val;

}

ParameterValue admplug::CompositeMapping::reverseMap(ParameterValue val) const
{
    for(int i = (mappings.size() - 1); i >= 0; i--) {
        val = mappings[i]->reverseMap(val);
    }
    return val;

}

void CompositeMapping::addMapping(std::shared_ptr<ParameterValueMapping> mapping)
{
    mappings.push_back(mapping);
}


ParameterRange::ParameterRange(double min, double max) :min{min}, max{max}
{
}

std::shared_ptr<ParameterValueMapping const> ParameterRange::modulus() const {
    auto minimum = std::min(min, max);
    auto maximum = std::max(min, max);
    return std::make_shared<FunctionalMapping>(
        [minimum, maximum](ParameterValue val){
        if(val.get() == maximum || val.get() == minimum) {
            // Prevents values right at extremities getting wrapped
            return val;
        }
        auto range = maximum - minimum;
        return ParameterValue{fmod(val.get() - minimum, range) + minimum, val.clipped};
    },
        [](ParameterValue val) {
        // Can't properly undo without original data
        return val;
    }
    );
}

std::shared_ptr<ParameterValueMapping const> ParameterRange::clipper() const {
    auto minimum = min;
    auto maximum = max;
    return std::make_shared<FunctionalMapping>(
        [minimum, maximum](ParameterValue val) {
        auto clipped = std::max(val.get(), minimum);
        clipped = std::min(clipped, maximum);
        return ParameterValue{clipped, (val.get() < minimum) || (val.get() > maximum)};
    },
        [](ParameterValue val) {
        // Can't properly undo without original data
        return val;
    }
    );
}

std::shared_ptr<ParameterValueMapping const> ParameterRange::normaliser() const {
    auto minimum = min;
    auto maximum = max;
    return std::make_shared<FunctionalMapping>(
        [minimum, maximum](ParameterValue val) {
        auto range = maximum - minimum;
        return ParameterValue((val.get() - minimum) / range, val.clipped);
    },
        [minimum, maximum](ParameterValue val) {
        auto range = maximum - minimum;
        return ParameterValue((val.get() * range) + minimum, val.clipped);
    }
    );
}


ParameterValue admplug::Inversion::forwardMap(ParameterValue val) const
{
    return ParameterValue(-val.get(), val.clipped);
}

ParameterValue admplug::Inversion::reverseMap(ParameterValue val) const
{
    return ParameterValue(-val.get(), val.clipped);
}



using MappingPtr = std::shared_ptr<ParameterValueMapping const>;
std::shared_ptr<CompositeMapping> admplug::getCombinedMapping(std::initializer_list<MappingPtr> list) {
    std::vector<MappingPtr> mappings{ list };
    return std::make_shared<CompositeMapping>(mappings);
}



std::shared_ptr<const ParameterValueMapping> map::sequence(std::initializer_list<std::shared_ptr<const ParameterValueMapping> > mappings) {
    return std::make_shared<CompositeMapping>(std::move(mappings));
}

std::shared_ptr<const ParameterValueMapping> map::normalise(const ParameterRange &range) {
    return range.normaliser();
}

std::shared_ptr<const ParameterValueMapping> map::clip() {
    auto clipRange = ParameterRange{0.0, 1.0};
    return clipRange.clipper();
}

std::shared_ptr<const ParameterValueMapping> map::linearToDb(const ParameterRange& range) {
    auto toDb = std::make_shared<LinearToDb>();
    return map::sequence({toDb, range.normaliser(), range.clipper()});
}

std::shared_ptr<const ParameterValueMapping> map::clip(const ParameterRange &range) {
    return range.clipper();
}

std::shared_ptr<const ParameterValueMapping> map::wrap(const ParameterRange &range) {
    return range.modulus();
}

std::shared_ptr<const ParameterValueMapping> map::invert() {
    return std::make_shared<Inversion>();
}



ParameterValue LinearToDb::forwardMap(ParameterValue val) const
{
  double db(0);
    if(val.get() <=0.00001) {
      db = -100;
    } else {
      db = 20.0 * log10(val.get());
    }
    return ParameterValue(db, val.clipped);
}

ParameterValue LinearToDb::reverseMap(ParameterValue val) const
{
    return ParameterValue(pow(10, val.get()/20), val.clipped);
}
