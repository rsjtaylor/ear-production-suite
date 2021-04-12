#ifndef AUTOMATIONPOINT_H
#define AUTOMATIONPOINT_H
#include <chrono>

namespace admplug {
class ParameterValue {
public:
  ParameterValue(double value) : value{value}, clipped{false} {}
  ParameterValue(double value, bool clipped) : value{value}, clipped{clipped} {}
  ParameterValue(const ParameterValue& obj) : value{obj.value}, clipped{obj.clipped} {}
  double get() const { return value; }
  operator double() const { return get(); }
 
  bool clipped;

private:
  double value;
  
};


class AutomationPoint
{
public:
  explicit AutomationPoint(double val);
    AutomationPoint(std::chrono::nanoseconds timeNs, std::chrono::nanoseconds duration, double val);
    AutomationPoint(double timeSeconds, double duration, ParameterValue val);
    double time() const;
    double duration() const;
    ParameterValue value() const;
private:
    double pointTime;
    double pointDuration;
    ParameterValue pointValue;
};

}

#endif // AUTOMATIONPOINT_H
