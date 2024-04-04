#ifndef BASIC_CONGESTION_POLICY_H
#define BASIC_CONGESTION_POLICY_H

namespace ns3
{

class BasicCongestionPolicy
{

  public:
    static int usage_percentage;

    static bool isCongested(ns3::Queue<ns3::Packet>* queue);
};

int BasicCongestionPolicy::usage_percentage = 0;

bool BasicCongestionPolicy::isCongested(ns3::Queue<ns3::Packet>* queue)
{
    NS_LOG_DEBUG("Queue size: " << queue->GetMaxSize().GetValue());
    NS_LOG_DEBUG("Queue packets: " << queue->GetNPackets());
    return queue->GetNPackets() * 100 >=
           queue->GetMaxSize().GetValue() * usage_percentage;
}

} // namespace ns3

#endif
