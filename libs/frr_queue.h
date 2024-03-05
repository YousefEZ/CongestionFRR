#ifndef FRR_QUEUE_HPP
#define FRR_QUEUE_HPP

#include <iostream>
#include <utility>

#include <typeinfo>
#include <cxxabi.h> // For __cxa_demangle

#include "ns3/object-base.h"
#include "ns3/queue.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-net-device.h"
#include <ns3/boolean.h>

#define STRINGIFY_TYPE_ALIAS(alias) typeid(alias).name()

namespace ns3
{

template <typename CONGESTION_POLICY, typename FRR_POLICY>
class FRRQueue : public Queue<Packet>
{
  private:    

    using Queue<Packet>::GetContainer;
    using Queue<Packet>::DoEnqueue;
    using Queue<Packet>::DoDequeue;
    using Queue<Packet>::DoRemove;
    using Queue<Packet>::DoPeek;
    
    NS_LOG_TEMPLATE_DECLARE;
    

    void ForwardToAlternateTarget(Ptr<Packet> packet);
    static std::string makeQueueString();
  protected:
    virtual bool Enqueue(Ptr<Packet> p) override; 
    virtual Ptr<Packet> Dequeue() override;
    virtual Ptr<Packet> Remove(void) override;
    virtual Ptr<const Packet> Peek() const override;
  public:
    
    FRR_POLICY m_frrPolicy;
    CONGESTION_POLICY m_congestionPolicy;
    
    static TypeId GetTypeId();
    FRRQueue();
    ~FRRQueue();
  
    template <typename... DEVICES>
    void addAlternateTargets(DEVICES&&... devices);

    static const std::string& getQueueString();
};


template <typename CONGESTION_POLICY, typename FRR_POLICY>
FRRQueue<CONGESTION_POLICY, FRR_POLICY>::FRRQueue()
    : Queue<Packet>(),
      NS_LOG_TEMPLATE_DEFINE("FRRQueue")
{
    NS_LOG_FUNCTION(this);
}


template <typename CONGESTION_POLICY, typename FRR_POLICY>
FRRQueue<CONGESTION_POLICY, FRR_POLICY>::~FRRQueue()
{
    NS_LOG_FUNCTION(this);
}

template <typename CONGESTION_POLICY, typename FRR_POLICY>
TypeId FRRQueue<CONGESTION_POLICY, FRR_POLICY>::GetTypeId()
{
    static TypeId tid = TypeId(getQueueString())
        .SetParent<Queue<Packet>>()
        .SetGroupName("Network")
        .template AddConstructor<FRRQueue<CONGESTION_POLICY, FRR_POLICY>>()
        .AddAttribute("MaxSize",
                      "The max queue size",
                      QueueSizeValue(QueueSize("100p")),
                      MakeQueueSizeAccessor(&QueueBase::SetMaxSize, &QueueBase::GetMaxSize),
                      MakeQueueSizeChecker());
    return tid;
}



template <typename CONGESTION_POLICY, typename FRR_POLICY>
bool FRRQueue<CONGESTION_POLICY, FRR_POLICY>::Enqueue(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    if (m_congestionPolicy.isCongested(GetContainer()) || !DoEnqueue(GetContainer().end(), packet))
    {
        NS_LOG_LOGIC("Congested Route, Rerouting packet: " << packet);
        ForwardToAlternateTarget(packet);
        NS_LOG_LOGIC("Rerouting complete");
        return false; 
    }
    NS_LOG_LOGIC("Enqueued " << packet << " to " << GetContainer().size() << " packets in queue.");
    return true;
}
 
template <typename CONGESTION_POLICY, typename FRR_POLICY>
Ptr<Packet> FRRQueue<CONGESTION_POLICY, FRR_POLICY>::Dequeue()
{
    NS_LOG_FUNCTION(this);
 
    Ptr<Packet> packet = DoDequeue(GetContainer().begin());
 
    NS_LOG_LOGIC("Popped " << packet);
    return packet;
}
 
template <typename CONGESTION_POLICY, typename FRR_POLICY>
Ptr<Packet> FRRQueue<CONGESTION_POLICY, FRR_POLICY>::Remove()
{
    NS_LOG_FUNCTION(this);
 
    Ptr<Packet> packet = DoRemove(GetContainer().begin());
 
    NS_LOG_LOGIC("Removed " << packet);
 
    return packet;
}
 
template <typename CONGESTION_POLICY, typename FRR_POLICY>
Ptr<const Packet> FRRQueue<CONGESTION_POLICY, FRR_POLICY>::Peek() const
{
    NS_LOG_FUNCTION(this);
 
    return DoPeek(GetContainer().begin());
}


template <typename CONGESTION_POLICY, typename FRR_POLICY>
void FRRQueue<CONGESTION_POLICY, FRR_POLICY>::ForwardToAlternateTarget(Ptr<Packet> packet)
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

template <typename CONGESTION_POLICY, typename FRR_POLICY>
std::string FRRQueue<CONGESTION_POLICY, FRR_POLICY>::makeQueueString()
{
    using QueueType = FRRQueue<CONGESTION_POLICY, FRR_POLICY>;   
    int status;
    char* demangled = abi::__cxa_demangle(STRINGIFY_TYPE_ALIAS(QueueType), nullptr, nullptr, &status);
    std::string result = (status == 0 && demangled != nullptr) ? demangled : STRINGIFY_TYPE_ALIAS(QueueType);
    free(demangled); 
    return result;
}

template <typename CONGESTION_POLICY, typename FRR_POLICY>
const std::string& FRRQueue<CONGESTION_POLICY, FRR_POLICY>::getQueueString()
{
  const static std::string result = FRRQueue<CONGESTION_POLICY, FRR_POLICY>::makeQueueString();
  return result;
}

NS_LOG_COMPONENT_DEFINE("FRRQueue");

} // namespace ns3

#endif 
