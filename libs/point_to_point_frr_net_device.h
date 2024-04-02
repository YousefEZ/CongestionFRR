/*
 * Copyright (c) 2007, 2008 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef POINT_TO_POINT_FRR_NET_DEVICE_H
#define POINT_TO_POINT_FRR_NET_DEVICE_H

#include "ns3/address.h"
#include "ns3/callback.h"
#include "ns3/data-rate.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/queue-fwd.h"
#include "ns3/traced-callback.h"

#include "ns3/ppp-header.h"

#include "ns3/error-model.h"
#include "ns3/llc-snap-header.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/pointer.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/channel.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"

#include "frr_queue_base.h"

#include <list>

#include <cstring>

#define STRINGIFY_TYPE_ALIAS(alias) typeid(alias).name()

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("FRRQueue");

template <typename FRR_POLICY>
class PointToPointFRRChannel;

class ErrorModel;

template <typename FRR_POLICY>
class PointToPointFRRNetDevice : public NetDevice
{
  public:
    static TypeId GetTypeId();

    PointToPointFRRNetDevice();

    ~PointToPointFRRNetDevice() override;

    // Delete copy constructor and assignment operator to avoid misuse
    PointToPointFRRNetDevice<FRR_POLICY>&
    operator=(const PointToPointFRRNetDevice<FRR_POLICY>&) = delete;
    PointToPointFRRNetDevice(const PointToPointFRRNetDevice<FRR_POLICY>&) =
        delete;

    void SetDataRate(DataRate bps);

    void SetInterframeGap(Time t);

    bool Attach(Ptr<PointToPointFRRChannel<FRR_POLICY>> ch);

    void SetQueue(Ptr<Queue<Packet>> queue);

    Ptr<Queue<Packet>> GetQueue() const;

    void SetReceiveErrorModel(Ptr<ErrorModel> em);

    void Receive(Ptr<Packet> p);

    // The remaining methods are documented in ns3::NetDevice*

    void SetIfIndex(const uint32_t index) override;
    uint32_t GetIfIndex() const override;

    Ptr<Channel> GetChannel() const override;

    void SetAddress(Address address) override;
    Address GetAddress() const override;

    bool SetMtu(const uint16_t mtu) override;
    uint16_t GetMtu() const override;

    bool IsLinkUp() const override;

    void AddLinkChangeCallback(Callback<void> callback) override;

    bool IsBroadcast() const override;
    Address GetBroadcast() const override;

    bool IsMulticast() const override;
    Address GetMulticast(Ipv4Address multicastGroup) const override;

    bool IsPointToPoint() const override;
    bool IsBridge() const override;

    bool Send(Ptr<Packet> packet, const Address& dest,
              uint16_t protocolNumber) override;
    bool SendFrom(Ptr<Packet> packet, const Address& source,
                  const Address& dest, uint16_t protocolNumber) override;

    Ptr<Node> GetNode() const override;
    void SetNode(Ptr<Node> node) override;

    bool NeedsArp() const override;

    void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;

    Address GetMulticast(Ipv6Address addr) const override;

    void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
    bool SupportsSendFrom() const override;

    bool isCongested();

    void addAlternateTarget(Ptr<ns3::NetDevice> device);

    static std::string makeNetDeviceString();
    static const std::string& getNetDeviceString();

  protected:
    void DoMpiReceive(Ptr<Packet> p);

  private:
    void DoDispose() override;

    Address GetRemote() const;

    void AddHeader(Ptr<Packet> p, uint16_t protocolNumber);

    bool ProcessHeader(Ptr<Packet> p, uint16_t& param);

    bool TransmitStart(Ptr<Packet> p);

    void TransmitComplete();

    void NotifyLinkUp();

    enum TxMachineState { READY, BUSY };

    TxMachineState m_txMachineState;

    DataRate m_bps;

    Time m_tInterframeGap;

    Ptr<PointToPointFRRChannel<FRR_POLICY>> m_channel;

    Ptr<Queue<Packet>> m_queue;

    Ptr<ErrorModel> m_receiveErrorModel;

    TracedCallback<Ptr<const Packet>> m_macTxTrace;

    TracedCallback<Ptr<const Packet>> m_macTxDropTrace;

    TracedCallback<Ptr<const Packet>> m_macPromiscRxTrace;

    TracedCallback<Ptr<const Packet>> m_macRxTrace;

    TracedCallback<Ptr<const Packet>> m_macRxDropTrace;

    TracedCallback<Ptr<const Packet>> m_phyTxBeginTrace;

    TracedCallback<Ptr<const Packet>> m_phyTxEndTrace;

    TracedCallback<Ptr<const Packet>> m_phyTxDropTrace;

    TracedCallback<Ptr<const Packet>> m_phyRxBeginTrace;

    TracedCallback<Ptr<const Packet>> m_phyRxEndTrace;

    TracedCallback<Ptr<const Packet>> m_phyRxDropTrace;

    TracedCallback<Ptr<const Packet>> m_snifferTrace;

    TracedCallback<Ptr<const Packet>> m_promiscSnifferTrace;

    Ptr<Node> m_node;
    Mac48Address m_address;
    NetDevice::ReceiveCallback m_rxCallback;
    NetDevice::PromiscReceiveCallback m_promiscCallback;
    //   (promisc data)
    uint32_t m_ifIndex;
    bool m_linkUp;
    TracedCallback<> m_linkChangeCallbacks;

    static const uint16_t DEFAULT_MTU = 1500;

    uint32_t m_mtu;

    Ptr<Packet> m_currentPkt;

    FRR_POLICY m_frr_policy;

    static uint16_t PppToEther(uint16_t protocol);

    static uint16_t EtherToPpp(uint16_t protocol);
};

class Packet;

template <typename FRR_POLICY>
class PointToPointFRRChannel : public Channel
{
  public:
    static TypeId GetTypeId();

    PointToPointFRRChannel();

    void Attach(Ptr<PointToPointFRRNetDevice<FRR_POLICY>> device);

    virtual bool TransmitStart(Ptr<const Packet> p,
                               Ptr<PointToPointFRRNetDevice<FRR_POLICY>> src,
                               Time txTime);

    std::size_t GetNDevices() const override;

    Ptr<PointToPointFRRNetDevice<FRR_POLICY>>
    GetPointToPointDevice(std::size_t i) const;

    Ptr<NetDevice> GetDevice(std::size_t i) const override;

    static std::string makeChannelString();
    static const std::string& getChannelString();

  protected:
    Time GetDelay() const;

    bool IsInitialized() const;

    Ptr<PointToPointFRRNetDevice<FRR_POLICY>> GetSource(uint32_t i) const;

    Ptr<PointToPointFRRNetDevice<FRR_POLICY>> GetDestination(uint32_t i) const;

    typedef void (*TxRxAnimationCallback)(Ptr<const Packet> packet,
                                          Ptr<NetDevice> txDevice,
                                          Ptr<NetDevice> rxDevice,
                                          Time duration, Time lastBitTime);

  private:
    static const std::size_t N_DEVICES = 2;

    Time m_delay;
    std::size_t m_nDevices;

    TracedCallback<Ptr<const Packet>, // Packet being transmitted
                   Ptr<NetDevice>,    // Transmitting NetDevice
                   Ptr<NetDevice>,    // Receiving NetDevice
                   Time,              // Amount of time to transmit the pkt
                   Time               // Last bit receive time (relative to now)
                   >
        m_txrxPointToPoint;

    enum WireState { INITIALIZING, IDLE, TRANSMITTING, PROPAGATING };

    class Link
    {
      public:
        Link() = default;

        WireState m_state{INITIALIZING};
        Ptr<PointToPointFRRNetDevice<FRR_POLICY>> m_src;
        Ptr<PointToPointFRRNetDevice<FRR_POLICY>> m_dst;
    };

    Link m_link[N_DEVICES];
};

template <typename FRR_POLICY>
TypeId PointToPointFRRChannel<FRR_POLICY>::GetTypeId()
{
    static TypeId tid =
        TypeId(getChannelString())
            .SetParent<Channel>()
            .SetGroupName("PointToPoint")
            .AddConstructor<PointToPointFRRChannel<FRR_POLICY>>()
            .AddAttribute(
                "Delay", "Propagation delay through the channel",
                TimeValue(Seconds(0)),
                MakeTimeAccessor(&PointToPointFRRChannel<FRR_POLICY>::m_delay),
                MakeTimeChecker())
            .AddTraceSource(
                "TxRxPointToPoint",
                "Trace source indicating transmission of packet "
                "from the PointToPointFRRChannel, used by the Animation "
                "interface.",
                MakeTraceSourceAccessor(
                    &PointToPointFRRChannel<FRR_POLICY>::m_txrxPointToPoint),
                getChannelString() + "::TxRxAnimationCallback");
    return tid;
}

//
// By default, you get a channel that
// has an "infitely" fast transmission speed and zero delay.
template <typename FRR_POLICY>
PointToPointFRRChannel<FRR_POLICY>::PointToPointFRRChannel()
    : Channel(), m_delay(Seconds(0.)), m_nDevices(0)
{
    NS_LOG_FUNCTION_NOARGS();
}

template <typename FRR_POLICY>
void PointToPointFRRChannel<FRR_POLICY>::Attach(
    Ptr<PointToPointFRRNetDevice<FRR_POLICY>> device)
{
    NS_LOG_FUNCTION(this << device);
    NS_ASSERT_MSG(m_nDevices < N_DEVICES, "Only two devices permitted");
    NS_ASSERT(device);

    m_link[m_nDevices++].m_src = device;
    //
    // If we have both devices connected to the channel, then finish introducing
    // the two halves and set the links to IDLE.
    //
    if (m_nDevices == N_DEVICES) {
        m_link[0].m_dst = m_link[1].m_src;
        m_link[1].m_dst = m_link[0].m_src;
        m_link[0].m_state = IDLE;
        m_link[1].m_state = IDLE;
    }
}

template <typename FRR_POLICY>
bool PointToPointFRRChannel<FRR_POLICY>::TransmitStart(
    Ptr<const Packet> p, Ptr<PointToPointFRRNetDevice<FRR_POLICY>> src,
    Time txTime)
{
    NS_LOG_FUNCTION(this << p << src);
    NS_LOG_LOGIC("UID is " << p->GetUid() << ")");

    NS_ASSERT(m_link[0].m_state != INITIALIZING);
    NS_ASSERT(m_link[1].m_state != INITIALIZING);

    uint32_t wire = src == m_link[0].m_src ? 0 : 1;

    Simulator::ScheduleWithContext(
        m_link[wire].m_dst->GetNode()->GetId(), txTime + m_delay,
        &PointToPointFRRNetDevice<FRR_POLICY>::Receive, m_link[wire].m_dst,
        p->Copy());

    // Call the tx anim callback on the net device
    m_txrxPointToPoint(p, src, m_link[wire].m_dst, txTime, txTime + m_delay);
    return true;
}

template <typename FRR_POLICY>
std::size_t PointToPointFRRChannel<FRR_POLICY>::GetNDevices() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_nDevices;
}

template <typename FRR_POLICY>
Ptr<PointToPointFRRNetDevice<FRR_POLICY>>
PointToPointFRRChannel<FRR_POLICY>::GetPointToPointDevice(std::size_t i) const
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(i < 2);
    return m_link[i].m_src;
}

template <typename FRR_POLICY>
Ptr<NetDevice>
PointToPointFRRChannel<FRR_POLICY>::GetDevice(std::size_t i) const
{
    NS_LOG_FUNCTION_NOARGS();
    return GetPointToPointDevice(i);
}

template <typename FRR_POLICY>
Time PointToPointFRRChannel<FRR_POLICY>::GetDelay() const
{
    return m_delay;
}

template <typename FRR_POLICY>
Ptr<PointToPointFRRNetDevice<FRR_POLICY>>
PointToPointFRRChannel<FRR_POLICY>::GetSource(uint32_t i) const
{
    return m_link[i].m_src;
}

template <typename FRR_POLICY>
Ptr<PointToPointFRRNetDevice<FRR_POLICY>>
PointToPointFRRChannel<FRR_POLICY>::GetDestination(uint32_t i) const
{
    return m_link[i].m_dst;
}

template <typename FRR_POLICY>
bool PointToPointFRRChannel<FRR_POLICY>::IsInitialized() const
{
    NS_ASSERT(m_link[0].m_state != INITIALIZING);
    NS_ASSERT(m_link[1].m_state != INITIALIZING);
    return true;
}

template <typename FRR_POLICY>
TypeId PointToPointFRRNetDevice<FRR_POLICY>::GetTypeId()
{
    static TypeId tid =
        TypeId(getNetDeviceString())
            .SetParent<NetDevice>()
            .SetGroupName("PointToPoint")
            .AddConstructor<PointToPointFRRNetDevice<FRR_POLICY>>()
            .AddAttribute("Mtu", "The MAC-level Maximum Transmission Unit",
                          UintegerValue(DEFAULT_MTU),
                          MakeUintegerAccessor(
                              &PointToPointFRRNetDevice<FRR_POLICY>::SetMtu,
                              &PointToPointFRRNetDevice<FRR_POLICY>::GetMtu),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("Address", "The MAC address of this device.",
                          Mac48AddressValue(Mac48Address("ff:ff:ff:ff:ff:ff")),
                          MakeMac48AddressAccessor(
                              &PointToPointFRRNetDevice<FRR_POLICY>::m_address),
                          MakeMac48AddressChecker())
            .AddAttribute("DataRate",
                          "The default data rate for point to point links",
                          DataRateValue(DataRate("32768b/s")),
                          MakeDataRateAccessor(
                              &PointToPointFRRNetDevice<FRR_POLICY>::m_bps),
                          MakeDataRateChecker())
            .AddAttribute(
                "ReceiveErrorModel",
                "The receiver error model used to simulate packet loss",
                PointerValue(),
                MakePointerAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_receiveErrorModel),
                MakePointerChecker<ErrorModel>())
            .AddAttribute(
                "InterframeGap",
                "The time to wait between packet (frame) transmissions",
                TimeValue(Seconds(0.0)),
                MakeTimeAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_tInterframeGap),
                MakeTimeChecker())

            //
            // Transmit queueing discipline for the device which includes its
            // own set of trace hooks.
            //
            .AddAttribute("TxQueue",
                          "A queue to use as the transmit queue in the device.",
                          PointerValue(),
                          MakePointerAccessor(
                              &PointToPointFRRNetDevice<FRR_POLICY>::m_queue),
                          MakePointerChecker<Queue<Packet>>())

            //
            // Trace sources at the "top" of the net device, where packets
            // transition to/from higher layers.
            //
            .AddTraceSource(
                "MacTx",
                "Trace source indicating a packet has arrived "
                "for transmission by this device",
                MakeTraceSourceAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_macTxTrace),
                "ns3::Packet::TracedCallback")
            .AddTraceSource(
                "MacTxDrop",
                "Trace source indicating a packet has been dropped "
                "by the device before transmission",
                MakeTraceSourceAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_macTxDropTrace),
                "ns3::Packet::TracedCallback")
            .AddTraceSource(
                "MacPromiscRx",
                "A packet has been received by this device, "
                "has been passed up from the physical layer "
                "and is being forwarded up the local protocol stack.  "
                "This is a promiscuous trace,",
                MakeTraceSourceAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_macPromiscRxTrace),
                "ns3::Packet::TracedCallback")
            .AddTraceSource(
                "MacRx",
                "A packet has been received by this device, "
                "has been passed up from the physical layer "
                "and is being forwarded up the local protocol stack.  "
                "This is a non-promiscuous trace,",
                MakeTraceSourceAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_macRxTrace),
                "ns3::Packet::TracedCallback")
#if 0
    // Not currently implemented for this device
    .AddTraceSource ("MacRxDrop",
                     "Trace source indicating a packet was dropped "
                     "before being forwarded up the stack",
                     MakeTraceSourceAccessor (&PointToPointFRRNetDevice<FRR_POLICY>::m_macRxDropTrace),
                     "ns3::Packet::TracedCallback")
#endif
            //
            // Trace sources at the "bottom" of the net device, where packets
            // transition to/from the channel.
            //
            .AddTraceSource(
                "PhyTxBegin",
                "Trace source indicating a packet has begun "
                "transmitting over the channel",
                MakeTraceSourceAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_phyTxBeginTrace),
                "ns3::Packet::TracedCallback")
            .AddTraceSource(
                "PhyTxEnd",
                "Trace source indicating a packet has been "
                "completely transmitted over the channel",
                MakeTraceSourceAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_phyTxEndTrace),
                "ns3::Packet::TracedCallback")
            .AddTraceSource(
                "PhyTxDrop",
                "Trace source indicating a packet has been "
                "dropped by the device during transmission",
                MakeTraceSourceAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_phyTxDropTrace),
                "ns3::Packet::TracedCallback")
#if 0
    // Not currently implemented for this device
    .AddTraceSource ("PhyRxBegin",
                     "Trace source indicating a packet has begun "
                     "being received by the device",
                     MakeTraceSourceAccessor (&PointToPointFRRNetDevice<FRR_POLICY>::m_phyRxBeginTrace),
                     "ns3::Packet::TracedCallback")
#endif
            .AddTraceSource(
                "PhyRxEnd",
                "Trace source indicating a packet has been "
                "completely received by the device",
                MakeTraceSourceAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_phyRxEndTrace),
                "ns3::Packet::TracedCallback")
            .AddTraceSource(
                "PhyRxDrop",
                "Trace source indicating a packet has been "
                "dropped by the device during reception",
                MakeTraceSourceAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_phyRxDropTrace),
                "ns3::Packet::TracedCallback")

            //
            // Trace sources designed to simulate a packet sniffer facility
            // (tcpdump). Note that there is really no difference between
            // promiscuous and non-promiscuous traces in a point-to-point link.
            //
            .AddTraceSource(
                "Sniffer",
                "Trace source simulating a non-promiscuous packet sniffer "
                "attached to the device",
                MakeTraceSourceAccessor(
                    &PointToPointFRRNetDevice<FRR_POLICY>::m_snifferTrace),
                "ns3::Packet::TracedCallback")
            .AddTraceSource(
                "PromiscSniffer",
                "Trace source simulating a promiscuous packet sniffer "
                "attached to the device",
                MakeTraceSourceAccessor(&PointToPointFRRNetDevice<
                                        FRR_POLICY>::m_promiscSnifferTrace),
                "ns3::Packet::TracedCallback");
    return tid;
}

template <typename FRR_POLICY>
PointToPointFRRNetDevice<FRR_POLICY>::PointToPointFRRNetDevice()
    : m_txMachineState(READY), m_channel(nullptr), m_linkUp(false),
      m_currentPkt(nullptr)
{
    NS_LOG_FUNCTION(this);
}

template <typename FRR_POLICY>
PointToPointFRRNetDevice<FRR_POLICY>::~PointToPointFRRNetDevice()
{
    NS_LOG_FUNCTION(this);
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::AddHeader(Ptr<Packet> p,
                                                     uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << p << protocolNumber);
    PppHeader ppp;
    ppp.SetProtocol(EtherToPpp(protocolNumber));
    p->AddHeader(ppp);
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::ProcessHeader(Ptr<Packet> p,
                                                         uint16_t& param)
{
    NS_LOG_FUNCTION(this << p << param);
    PppHeader ppp;
    p->RemoveHeader(ppp);
    param = PppToEther(ppp.GetProtocol());
    return true;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_node = nullptr;
    m_channel = nullptr;
    m_receiveErrorModel = nullptr;
    m_currentPkt = nullptr;
    m_queue = nullptr;
    NetDevice::DoDispose();
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::SetDataRate(DataRate bps)
{
    NS_LOG_FUNCTION(this);
    m_bps = bps;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::SetInterframeGap(Time t)
{
    NS_LOG_FUNCTION(this << t.As(Time::S));
    m_tInterframeGap = t;
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::TransmitStart(Ptr<Packet> p)
{
    NS_LOG_FUNCTION(this << p);
    NS_LOG_LOGIC("UID is " << p->GetUid() << ")");

    //
    // This function is called to start the process of transmitting a packet.
    // We need to tell the channel that we've started wiggling the wire and
    // schedule an event that will be executed when the transmission is
    // complete.
    //
    NS_ASSERT_MSG(m_txMachineState == READY, "Must be READY to transmit");
    m_txMachineState = BUSY;
    m_currentPkt = p;
    m_phyTxBeginTrace(m_currentPkt);

    Time txTime = m_bps.CalculateBytesTxTime(p->GetSize());
    Time txCompleteTime = txTime + m_tInterframeGap;

    NS_LOG_LOGIC("Schedule TransmitCompleteEvent in "
                 << txCompleteTime.As(Time::S));
    Simulator::Schedule(txCompleteTime,
                        &PointToPointFRRNetDevice<FRR_POLICY>::TransmitComplete,
                        this);

    bool result = m_channel->TransmitStart(p, this, txTime);
    if (!result) {
        m_phyTxDropTrace(p);
    }
    return result;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::TransmitComplete()
{
    NS_LOG_FUNCTION(this);

    //
    // This function is called to when we're all done transmitting a packet.
    // We try and pull another packet off of the transmit queue.  If the queue
    // is empty, we are done, otherwise we need to start transmitting the
    // next packet.
    //
    NS_ASSERT_MSG(m_txMachineState == BUSY, "Must be BUSY if transmitting");
    m_txMachineState = READY;

    NS_ASSERT_MSG(
        m_currentPkt,
        "PointToPointFRRNetDevice::TransmitComplete(): m_currentPkt zero");

    m_phyTxEndTrace(m_currentPkt);
    m_currentPkt = nullptr;

    Ptr<Packet> p = m_queue->Dequeue();
    if (!p) {
        NS_LOG_LOGIC("No pending packets in device queue after tx complete");
        return;
    }

    //
    // Got another packet off of the queue, so start the transmit process again.
    //
    m_snifferTrace(p);
    m_promiscSnifferTrace(p);
    TransmitStart(p);
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::Attach(
    Ptr<PointToPointFRRChannel<FRR_POLICY>> ch)
{
    NS_LOG_FUNCTION(this << &ch);

    m_channel = ch;

    m_channel->Attach(this);

    //
    // This device is up whenever it is attached to a channel.  A better plan
    // would be to have the link come up when both devices are attached, but
    // this is not done for now.
    //
    NotifyLinkUp();
    return true;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::SetQueue(Ptr<Queue<Packet>> q)
{
    NS_LOG_FUNCTION(this << q);
    m_queue = q;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::SetReceiveErrorModel(
    Ptr<ErrorModel> em)
{
    NS_LOG_FUNCTION(this << em);
    m_receiveErrorModel = em;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::Receive(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    uint16_t protocol = 0;

    if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt(packet)) {
        //
        // If we have an error model and it indicates that it is time to lose a
        // corrupted packet, don't forward this packet up, let it go.
        //
        m_phyRxDropTrace(packet);
    } else {
        //
        // Hit the trace hooks.  All of these hooks are in the same place in
        // this device because it is so simple, but this is not usually the case
        // in more complicated devices.
        //
        m_snifferTrace(packet);
        m_promiscSnifferTrace(packet);
        m_phyRxEndTrace(packet);

        //
        // Trace sinks will expect complete packets, not packets without some of
        // the headers.
        //
        Ptr<Packet> originalPacket = packet->Copy();

        //
        // Strip off the point-to-point protocol header and forward this packet
        // up the protocol stack.  Since this is a simple point-to-point link,
        // there is no difference in what the promisc callback sees and what the
        // normal receive callback sees.
        //
        ProcessHeader(packet, protocol);

        if (!m_promiscCallback.IsNull()) {
            m_macPromiscRxTrace(originalPacket);
            m_promiscCallback(this, packet, protocol, GetRemote(), GetAddress(),
                              NetDevice::PACKET_HOST);
        }

        m_macRxTrace(originalPacket);
        m_rxCallback(this, packet, protocol, GetRemote());
    }
}

template <typename FRR_POLICY>
Ptr<Queue<Packet>> PointToPointFRRNetDevice<FRR_POLICY>::GetQueue() const
{
    NS_LOG_FUNCTION(this);
    return m_queue;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::NotifyLinkUp()
{
    NS_LOG_FUNCTION(this);
    m_linkUp = true;
    m_linkChangeCallbacks();
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::SetIfIndex(const uint32_t index)
{
    NS_LOG_FUNCTION(this);
    m_ifIndex = index;
}

template <typename FRR_POLICY>
uint32_t PointToPointFRRNetDevice<FRR_POLICY>::GetIfIndex() const
{
    return m_ifIndex;
}

template <typename FRR_POLICY>
Ptr<Channel> PointToPointFRRNetDevice<FRR_POLICY>::GetChannel() const
{
    return m_channel;
}

//
// This is a point-to-point device, so we really don't need any kind of address
// information.  However, the base class NetDevice wants us to define the
// methods to get and set the address.  Rather than be rude and assert, we let
// clients get and set the address, but simply ignore them.

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::SetAddress(Address address)
{
    NS_LOG_FUNCTION(this << address);
    m_address = Mac48Address::ConvertFrom(address);
}

template <typename FRR_POLICY>
Address PointToPointFRRNetDevice<FRR_POLICY>::GetAddress() const
{
    return m_address;
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::IsLinkUp() const
{
    NS_LOG_FUNCTION(this);
    return m_linkUp;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::AddLinkChangeCallback(
    Callback<void> callback)
{
    NS_LOG_FUNCTION(this);
    m_linkChangeCallbacks.ConnectWithoutContext(callback);
}

//
// This is a point-to-point device, so every transmission is a broadcast to
// all of the devices on the network.
//
template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::IsBroadcast() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

//
// We don't really need any addressing information since this is a
// point-to-point device.  The base class NetDevice wants us to return a
// broadcast address, so we make up something reasonable.
//
template <typename FRR_POLICY>
Address PointToPointFRRNetDevice<FRR_POLICY>::GetBroadcast() const
{
    NS_LOG_FUNCTION(this);
    return Mac48Address::GetBroadcast();
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::IsMulticast() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

template <typename FRR_POLICY>
Address PointToPointFRRNetDevice<FRR_POLICY>::GetMulticast(
    Ipv4Address multicastGroup) const
{
    NS_LOG_FUNCTION(this);
    return Mac48Address("01:00:5e:00:00:00");
}

template <typename FRR_POLICY>
Address
PointToPointFRRNetDevice<FRR_POLICY>::GetMulticast(Ipv6Address addr) const
{
    NS_LOG_FUNCTION(this << addr);
    return Mac48Address("33:33:00:00:00:00");
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::IsPointToPoint() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::IsBridge() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::addAlternateTarget(
    Ptr<ns3::NetDevice> device)
{
    m_frr_policy.addAlternateTargets(device);
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::isCongested()
{
    return dynamic_cast<FRRQueueBase*>(PeekPointer(m_queue))->isCongested();
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::Send(Ptr<Packet> packet,
                                                const Address& dest,
                                                uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << dest << protocolNumber);
    NS_LOG_LOGIC("p=" << packet << ", dest=" << &dest);
    NS_LOG_LOGIC("UID is " << packet->GetUid());

    //
    // If IsLinkUp() is false it means there is no channel to send any packet
    // over so we just hit the drop trace on the packet and return an error.
    //
    if (!IsLinkUp()) {
        m_macTxDropTrace(packet);
        return false;
    }

    if (isCongested()) {
        NS_LOG_LOGIC("Device is congested, rerouting");
        return m_frr_policy.reroute(packet, dest, protocolNumber);
    }
    NS_LOG_LOGIC("Device is not congested, sending packet normally");
    //
    // Stick a point to point protocol header on the packet in preparation for
    // shoving it out the door.
    //

    AddHeader(packet, protocolNumber);
    m_macTxTrace(packet);

    //
    // We should enqueue and dequeue the packet to hit the tracing hooks.
    //
    if (m_queue->Enqueue(packet)) {
        //
        // If the channel is ready for transition we send the packet right now
        //
        if (m_txMachineState == READY) {
            packet = m_queue->Dequeue();
            m_snifferTrace(packet);
            m_promiscSnifferTrace(packet);
            bool ret = TransmitStart(packet);
            return ret;
        }
        return true;
    }

    // Enqueue may fail (overflow)

    m_macTxDropTrace(packet);
    return false;
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::SendFrom(Ptr<Packet> packet,
                                                    const Address& source,
                                                    const Address& dest,
                                                    uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << source << dest << protocolNumber);
    return false;
}

template <typename FRR_POLICY>
Ptr<Node> PointToPointFRRNetDevice<FRR_POLICY>::GetNode() const
{
    return m_node;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this);
    m_node = node;
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::NeedsArp() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::SetReceiveCallback(
    NetDevice::ReceiveCallback cb)
{
    m_rxCallback = cb;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::SetPromiscReceiveCallback(
    NetDevice::PromiscReceiveCallback cb)
{
    m_promiscCallback = cb;
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::SupportsSendFrom() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

template <typename FRR_POLICY>
void PointToPointFRRNetDevice<FRR_POLICY>::DoMpiReceive(Ptr<Packet> p)
{
    NS_LOG_FUNCTION(this << p);
    Receive(p);
}

template <typename FRR_POLICY>
Address PointToPointFRRNetDevice<FRR_POLICY>::GetRemote() const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_channel->GetNDevices() == 2);
    for (std::size_t i = 0; i < m_channel->GetNDevices(); ++i) {
        Ptr<NetDevice> tmp = m_channel->GetDevice(i);
        if (tmp != this) {
            return tmp->GetAddress();
        }
    }
    NS_ASSERT(false);
    // quiet compiler.
    return Address();
}

template <typename FRR_POLICY>
bool PointToPointFRRNetDevice<FRR_POLICY>::SetMtu(uint16_t mtu)
{
    NS_LOG_FUNCTION(this << mtu);
    m_mtu = mtu;
    return true;
}

template <typename FRR_POLICY>
uint16_t PointToPointFRRNetDevice<FRR_POLICY>::GetMtu() const
{
    NS_LOG_FUNCTION(this);
    return m_mtu;
}

template <typename FRR_POLICY>
uint16_t PointToPointFRRNetDevice<FRR_POLICY>::PppToEther(uint16_t proto)
{
    NS_LOG_FUNCTION_NOARGS();
    switch (proto) {
    case 0x0021: return 0x0800; // IPv4
    case 0x0057: return 0x86DD; // IPv6
    default: NS_ASSERT_MSG(false, "PPP Protocol number not defined!");
    }
    return 0;
}

template <typename FRR_POLICY>
uint16_t PointToPointFRRNetDevice<FRR_POLICY>::EtherToPpp(uint16_t proto)
{
    NS_LOG_FUNCTION_NOARGS();
    switch (proto) {
    case 0x0800: return 0x0021; // IPv4
    case 0x86DD: return 0x0057; // IPv6
    default: NS_ASSERT_MSG(false, "PPP Protocol number not defined!");
    }
    return 0;
}

template <typename FRR_POLICY>
std::string PointToPointFRRChannel<FRR_POLICY>::makeChannelString()
{
    using ChannelType = PointToPointFRRChannel<FRR_POLICY>;
    int status;
    char* demangled = abi::__cxa_demangle(STRINGIFY_TYPE_ALIAS(ChannelType),
                                          nullptr, nullptr, &status);
    std::string result = (status == 0 && demangled != nullptr)
                             ? demangled
                             : STRINGIFY_TYPE_ALIAS(ChannelType);
    free(demangled);
    return result;
}

template <typename FRR_POLICY>
const std::string& PointToPointFRRChannel<FRR_POLICY>::getChannelString()
{
    const static std::string result =
        PointToPointFRRChannel<FRR_POLICY>::makeChannelString();
    return result;
}

template <typename FRR_POLICY>
std::string PointToPointFRRNetDevice<FRR_POLICY>::makeNetDeviceString()
{
    using NetDeviceType = PointToPointFRRNetDevice<FRR_POLICY>;
    int status;
    char* demangled = abi::__cxa_demangle(STRINGIFY_TYPE_ALIAS(NetDeviceType),
                                          nullptr, nullptr, &status);
    std::string result = (status == 0 && demangled != nullptr)
                             ? demangled
                             : STRINGIFY_TYPE_ALIAS(NetDeviceType);
    free(demangled);
    return result;
}

template <typename FRR_POLICY>
const std::string& PointToPointFRRNetDevice<FRR_POLICY>::getNetDeviceString()
{
    const static std::string result =
        PointToPointFRRNetDevice<FRR_POLICY>::makeNetDeviceString();
    return result;
}

} // namespace ns3

#endif /* POINT_TO_POINT_FRR_NET_DEVICE_H */
