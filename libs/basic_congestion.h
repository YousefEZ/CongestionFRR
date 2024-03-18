#ifndef BASIC_CONGESTION_POLICY_H
#define BASIC_CONGESTION_POLICY_H

namespace ns3
{

template <int MAX_USAGE_PERCENTAGE>
class BasicCongestionPolicy
{
    static_assert(MAX_USAGE_PERCENTAGE > 0 && MAX_USAGE_PERCENTAGE <= 100,
                  "MAX_USAGE_PERCENTAGE must be between 1 and 100");

  public:
    static bool isCongested(ns3::Queue<ns3::Packet>* queue);
};

template <>
bool BasicCongestionPolicy<100>::isCongested(ns3::Queue<ns3::Packet>* queue)
{
    return queue->GetNPackets() >= queue->GetMaxSize().GetValue();
}

template <int MAX_USAGE_PERCENTAGE>
bool BasicCongestionPolicy<MAX_USAGE_PERCENTAGE>::isCongested(
    ns3::Queue<ns3::Packet>* queue)
{
    return queue->GetNPackets() * 100 >=
           queue->GetMaxSize().GetValue() * MAX_USAGE_PERCENTAGE;
}

} // namespace ns3

#endif
