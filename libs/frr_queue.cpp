#include "frr_queue.h"


bool CongestionPolicy::isCongested(const ns3::Queue<ns3::Packet> &queue)
{
    // Placeholder for congestion detection
    return false;
}

template <typename CONGESTION_POLICY>
FRRQueue<CONGESTION_POLICY>::FRRQueue() : ns3::Queue<ns3::Packet>{}
{
    NS_LOG_FUNCTION(this);
}

template <typename CONGESTION_POLICY>
ns3::TypeId FRRQueue<CONGESTION_POLICY>::GetTypeId(void)
{
    static ns3::TypeId tid = ns3::TypeId("FRRQueue")
        .SetParent<ns3::Queue<ns3::Packet>>()
        .SetGroupName("Network")
        .AddConstructor<FRRQueue>()
        .AddAttribute("AlternateTarget",
                       "The alternate PointToPointNetDevice for rerouting packets.",
                       ns3::PointerValue(),
                       ns3::MakePointerAccessor(&FRRQueue::m_alternateTarget),
                       ns3::MakePointerChecker<ns3::PointToPointNetDevice>())
        .AddAttribute("Alternate", "Alternate routing for every other packet.",
                       ns3::BooleanValue(false),
                       ns3::MakeBooleanAccessor(&FRRQueue::m_alternate),
                       ns3::MakeBooleanChecker());
    return tid;
}


template <typename CONGESTION_POLICY>
bool FRRQueue<CONGESTION_POLICY>::Enqueue(ns3::Ptr<ns3::Packet> p)
{
    NS_LOG_FUNCTION(this << p);

    if (CONGESTION_POLICY::isCongested(this))
    {
        ForwardToAlternateTarget(p);
        return true; 
    }
    return ns3::Queue<ns3::Packet>::Enqueue(p);
}

template <typename CONGESTION_POLICY>
void FRRQueue<CONGESTION_POLICY>::ForwardToAlternateTarget(ns3::Ptr<ns3::Packet> packet)
{
    m_alternateTarget->Send(packet, m_alternateTarget->GetAddress(), 0x0800);
}


