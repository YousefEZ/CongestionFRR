#ifndef FRR_QUEUE_BASE_H
#define FRR_QUEUE_BASE_H

#include "ns3/queue.h"
#include "ns3/packet.h"

namespace ns3
{

class FRRQueueBase : public Queue<Packet>
{
  protected:
    using Queue<Packet>::GetContainer;
    using Queue<Packet>::DoEnqueue;
    using Queue<Packet>::DoDequeue;
    using Queue<Packet>::DoRemove;
    using Queue<Packet>::DoPeek;
    using Queue<Packet>::GetNPackets;

  public:
    FRRQueueBase() = default;
    virtual ~FRRQueueBase() = default;

    virtual bool isCongested() = 0;
};

} // namespace ns3

#endif
