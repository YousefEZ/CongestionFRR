#ifndef LFA_POLICY_H
#define LFA_POLICY_H

#include <list>
#include <utility>

#include "ns3/queue.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-net-device.h"

namespace ns3
{

class LFAPolicy
{
  private:  
    std::list<ns3::Ptr<ns3::PointToPointNetDevice>> m_alternateTargets;

  public:
    LFAPolicy() = default;
 
    template <typename... DEVICES>
    void addAlternateTargets(DEVICES&&... devices);

    ns3::Ptr<ns3::PointToPointNetDevice> selectAlternativeTarget();
};


template <typename... DEVICES>
void LFAPolicy::addAlternateTargets(DEVICES&&... devices)
{
  (m_alternateTargets.push_back(std::forward<DEVICES>(devices)), ...); 
}

ns3::Ptr<ns3::PointToPointNetDevice> LFAPolicy::selectAlternativeTarget()
{
  if (m_alternateTargets.empty())
  {
    return nullptr;
  }
  return m_alternateTargets.front();
}


} // namespace ns3
  
#endif
