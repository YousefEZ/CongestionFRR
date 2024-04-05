#ifndef FRR_QUEUE_HPP
#define FRR_QUEUE_HPP

#include <iostream>
#include <utility>

#include <typeinfo>
#include <cxxabi.h>

#include "point_to_point_frr_net_device.h"
#include "ns3/object-base.h"
#include <ns3/boolean.h>
#include "frr_queue_base.h"

#define STRINGIFY_TYPE_ALIAS(alias) typeid(alias).name()

namespace ns3
{

template <typename CONGESTION_POLICY>
class FRRQueue : public FRRQueueBase
{
  public:
    int m_uid;

  private:
    using FRRQueueBase::DoDequeue;
    using FRRQueueBase::DoEnqueue;
    using FRRQueueBase::DoPeek;
    using FRRQueueBase::DoRemove;
    using FRRQueueBase::GetContainer;
    using FRRQueueBase::GetNPackets;

    NS_LOG_TEMPLATE_DECLARE;

    void ForwardToAlternateTarget(Ptr<Packet> packet);
    static std::string makeQueueString();

  protected:
    virtual bool Enqueue(Ptr<Packet> p) override;
    virtual Ptr<Packet> Dequeue() override;
    virtual Ptr<Packet> Remove(void) override;
    virtual Ptr<const Packet> Peek() const override;

  public:
    static int s_uid;
    CONGESTION_POLICY m_congestionPolicy;

    static TypeId GetTypeId();
    FRRQueue();
    virtual ~FRRQueue() = default;

    virtual bool isCongested() override;
    static const std::string& getQueueString();
};

template <typename CONGESTION_POLICY>
bool FRRQueue<CONGESTION_POLICY>::isCongested()
{
    return m_congestionPolicy.isCongested(this);
}

template <typename CONGESTION_POLICY>
int FRRQueue<CONGESTION_POLICY>::s_uid = 0;

template <typename CONGESTION_POLICY>
FRRQueue<CONGESTION_POLICY>::FRRQueue()
    : FRRQueueBase(), m_uid(s_uid++), NS_LOG_TEMPLATE_DEFINE("FRRQueue")
{
    NS_LOG_FUNCTION(this);
}

template <typename CONGESTION_POLICY>
TypeId FRRQueue<CONGESTION_POLICY>::GetTypeId()
{
    static TypeId tid =
        TypeId(getQueueString())
            .SetParent<Queue<Packet>>()
            .SetGroupName("Network")
            .template AddConstructor<FRRQueue<CONGESTION_POLICY>>()
            .AddAttribute("MaxSize", "The max queue size",
                          QueueSizeValue(QueueSize("10p")),
                          MakeQueueSizeAccessor(&QueueBase::SetMaxSize,
                                                &QueueBase::GetMaxSize),
                          MakeQueueSizeChecker());
    return tid;
}

template <typename CONGESTION_POLICY>
bool FRRQueue<CONGESTION_POLICY>::Enqueue(Ptr<Packet> packet)
{
    NS_LOG_LOGIC("(" << m_uid << ") Enqueuing: " << packet);
    NS_LOG_LOGIC(GetNPackets() << " packets in queue");
    return DoEnqueue(GetContainer().end(), packet);
}

template <typename CONGESTION_POLICY>
Ptr<Packet> FRRQueue<CONGESTION_POLICY>::Dequeue()
{
    // NS_LOG_FUNCTION(this);

    Ptr<Packet> packet = DoDequeue(GetContainer().begin());
    NS_LOG_LOGIC("(" << m_uid << ") popped " << packet);
    return packet;
}

template <typename CONGESTION_POLICY>
Ptr<Packet> FRRQueue<CONGESTION_POLICY>::Remove()
{
    NS_LOG_FUNCTION(this);

    Ptr<Packet> packet = DoRemove(GetContainer().begin());

    NS_LOG_LOGIC("Removed " << packet);

    return packet;
}

template <typename CONGESTION_POLICY>
Ptr<const Packet> FRRQueue<CONGESTION_POLICY>::Peek() const
{
    NS_LOG_FUNCTION(this);

    return DoPeek(GetContainer().begin());
}

template <typename CONGESTION_POLICY>
std::string FRRQueue<CONGESTION_POLICY>::makeQueueString()
{
    using QueueType = FRRQueue<CONGESTION_POLICY>;
    int status;
    char* demangled = abi::__cxa_demangle(STRINGIFY_TYPE_ALIAS(QueueType),
                                          nullptr, nullptr, &status);
    std::string result = (status == 0 && demangled != nullptr)
                             ? demangled
                             : STRINGIFY_TYPE_ALIAS(QueueType);
    free(demangled);
    return result;
}

template <typename CONGESTION_POLICY>
const std::string& FRRQueue<CONGESTION_POLICY>::getQueueString()
{
    const static std::string result =
        FRRQueue<CONGESTION_POLICY>::makeQueueString();
    return result;
}

} // namespace ns3

#endif
