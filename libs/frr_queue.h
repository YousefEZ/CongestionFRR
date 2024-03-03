#ifndef FRR_QUEUE_HPP
#define FRR_QUEUE_HPP

#include <iostream>
#include <utility>

#include "ns3/queue.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-net-device.h"
#include <ns3/boolean.h>

#define STRINGIFY_TYPE_ALIAS(alias) typeid(alias).name()

namespace ns3
{

template <typename CONGESTION_POLICY, typename FRR_POLICY>
class FRRQueue : public ns3::Queue<ns3::Packet>
{
  private:
    FRR_POLICY m_frrPolicy;
    void ForwardToAlternateTarget(ns3::Ptr<ns3::Packet> packet);
    static std::string makeQueueString();  
  protected:
    virtual bool Enqueue(ns3::Ptr<ns3::Packet> p); 
    virtual ns3::Ptr<ns3::Packet> Dequeue() override;
    virtual ns3::Ptr<ns3::Packet> Remove(void) override;
    virtual ns3::Ptr<const ns3::Packet> Peek() const override;
  public:
    static ns3::TypeId GetTypeId();
    FRRQueue() = default;
  
    template <typename... DEVICES>
    void addAlternateTargets(DEVICES&&... devices);

    static inline const std::string& getQueueString();
};


template <typename CONGESTION_POLICY, typename FRR_POLICY>
std::string FRRQueue<CONGESTION_POLICY, FRR_POLICY>::makeQueueString() 
{
  int status;
  using queueType = FRRQueue<CONGESTION_POLICY, FRR_POLICY>;
  char* demangled = abi::__cxa_demangle(STRINGIFY_TYPE_ALIAS(queueType), nullptr, nullptr, &status);
  std::string result = (status == 0 && demangled != nullptr) ? demangled : STRINGIFY_TYPE_ALIAS(queueType);
  free(demangled); // Free the memory allocated by __cxa_demangle
  return result;
}


template <typename CONGESTION_POLICY, typename FRR_POLICY>
const std::string& FRRQueue<CONGESTION_POLICY, FRR_POLICY>::getQueueString() 
{
  const static std::string queueString{makeQueueString()};
  return queueString;
}

/**
 *.AddAttribute("FRRPolicy",
                       "The alternate PointToPointNetDevice for rerouting packets.",
                       ns3::PointerValue(),
                       ns3::MakePointerAccessor(&FRRQueue<CONGESTION_POLICY, FRR_POLICY>::m_frrPolicy),
                       ns3::MakePointerChecker<ns3::PointToPointNetDevice>())
        .AddAttribute("Alternate", "Alternate routing for every other packet.",
                       ns3::BooleanValue(false),
                       ns3::MakeBooleanAccessor(&FRRQueue::m_alternate),
                       ns3::MakeBooleanChecker());
 *
 */

template <typename CONGESTION_POLICY, typename FRR_POLICY>
ns3::TypeId FRRQueue<CONGESTION_POLICY, FRR_POLICY>::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId(getQueueString())
        .SetParent<ns3::Queue<ns3::Packet>>()
        .SetGroupName("Network")
        .AddConstructor<FRRQueue<CONGESTION_POLICY, FRR_POLICY>>();
    return tid;
}



template <typename CONGESTION_POLICY, typename FRR_POLICY>
bool FRRQueue<CONGESTION_POLICY, FRR_POLICY>::Enqueue(ns3::Ptr<ns3::Packet> p)
{
    if (CONGESTION_POLICY::isCongested(this))
    {
        ForwardToAlternateTarget(p);
        return true; 
    }
    return ns3::Queue<ns3::Packet>::Enqueue(p);
}



template <typename CONGESTION_POLICY, typename FRR_POLICY>
void FRRQueue<CONGESTION_POLICY, FRR_POLICY>::ForwardToAlternateTarget(ns3::Ptr<ns3::Packet> packet)
{
  auto alternativeTarget = m_frrPolicy.selectAlternativeTarget();
  alternativeTarget->Send(packet, alternativeTarget->GetAddress(), 0x0800);
}

template <typename CONGESTION_POLICY, typename FRR_POLICY>
template <typename... DEVICES>
void FRRQueue<CONGESTION_POLICY, FRR_POLICY>::addAlternateTargets(DEVICES&&... devices)
{
  m_frrPolicy.addAlternateTargets(std::forward<DEVICES>(devices)...);
}


} // namespace ns3

#endif 
