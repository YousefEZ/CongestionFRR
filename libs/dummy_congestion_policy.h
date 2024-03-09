#ifndef DUMMY_CONGESTION_POLICY_H
#define DUMMY_CONGESTION_POLICY_H

#include "ns3/queue.h"
#include "ns3/packet.h"

namespace ns3
{

class DummyCongestionPolicy
{
  public:
    static bool isCongested(ns3::Queue<ns3::Packet>* queue);
};

bool DummyCongestionPolicy::isCongested(ns3::Queue<ns3::Packet>* queue)
{
    // Placeholder for congestion detection
    return false;
}

} // namespace ns3

#endif
