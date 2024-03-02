#ifndef FRR_QUEUE_HPP
#define FRR_QUEUE_HPP


#include "ns3/queue.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-net-device.h"
#include <ns3/boolean.h>



template <typename CONGESTION_POLICY>
class FRRQueue : public ns3::Queue<ns3::Packet>
{
  private:
    ns3::Ptr<ns3::PointToPointNetDevice> m_alternateTarget; // Device for alternate routing

    void ForwardToAlternateTarget(ns3::Ptr<ns3::Packet> packet);
  
  protected:
    virtual bool Enqueue(ns3::Ptr<ns3::Packet> p);
    //virtual ns3::Ptr<ns3::Packet> DoDequeue() override;
    //virtual ns3::Ptr<ns3::Packet> DoRemove(void) override;
    //virtual ns3::Ptr<const ns3::Packet> DoPeek() const override;
  public:
    static ns3::TypeId GetTypeId(void);
    FRRQueue();


};


class CongestionPolicy
{
  public:
    static bool isCongested(const ns3::Queue<ns3::Packet> &queue);
};

#endif 
