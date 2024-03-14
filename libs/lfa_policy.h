#ifndef LFA_POLICY_H
#define LFA_POLICY_H

#include <list>
#include <utility>

#include "ns3/queue.h"
#include "ns3/packet.h"
#include "point_to_point_frr_net_device.h"

namespace ns3
{

class LFAPolicy;

class LFAPolicy
{
  private:
    std::list<ns3::Ptr<ns3::PointToPointFRRNetDevice<LFAPolicy>>>
        m_alternateTargets;

  public:
    LFAPolicy() = default;

    template <typename... DEVICES>
    void addAlternateTargets(DEVICES&&... devices);

    bool reroute(Ptr<Packet> packet, const Address& dest,
                 uint16_t protocolNumber);
};

template <typename... DEVICES>
void LFAPolicy::addAlternateTargets(DEVICES&&... devices)
{
    (m_alternateTargets.push_back(std::forward<DEVICES>(devices)), ...);
}

bool LFAPolicy::reroute(Ptr<Packet> packet, const Address& dest,
                        uint16_t protocolNumber)
{
    for (auto& target : m_alternateTargets) {
        if (!target->isCongested()) {
            target->Send(packet, dest, protocolNumber);
            return true;
        }
    }
    return false;
}

} // namespace ns3

#endif
