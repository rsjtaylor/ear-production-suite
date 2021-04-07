#ifndef AUTOMATIONPOINT_H
#define AUTOMATIONPOINT_H
#include <chrono>

namespace admplug {
class ParameterValue {
public:
  ParameterValue(double value) : value{value}, clipped{false} {}
  ParameterValue(double value, bool clipped) : value{value}, clipped{clipped} {}
  double get() const { return value; }
  operator double() const { return get(); }
  bool wasClipped() const { return clipped; }
  void setClipped() { clipped = true; }

private:
  double value;
  bool clipped;
};


class AutomationPoint
{
public:
  explicit AutomationPoint(double val);
    AutomationPoint(std::chrono::nanoseconds timeNs, std::chrono::nanoseconds duration, double val);
    AutomationPoint(double timeSeconds, double duration, double val);
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
