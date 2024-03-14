#ifndef POINT_TO_POINT_FRR_HELPER_H
#define POINT_TO_POINT_FRR_HELPER_H

#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-channel.h"
#include "point_to_point_frr_net_device.h"
#include "ns3/simulator.h"

#include "ns3/trace-helper.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/queue.h"
#include "ns3/trace-helper.h"

#include <string>

namespace ns3
{

class NetDevice;
class Node;

template <typename FRR_POLICY>
class PointToPointFRRHelper : public PcapHelperForDevice,
                              public AsciiTraceHelperForDevice
{
  public:
    PointToPointFRRHelper();

    ~PointToPointFRRHelper() override
    {
    }

    template <typename... Ts>
    void SetQueue(std::string type, Ts&&... args);

    void SetDeviceAttribute(std::string name, const AttributeValue& value);

    void SetChannelAttribute(std::string name, const AttributeValue& value);

    void DisableFlowControl();

    NetDeviceContainer Install(NodeContainer c);

    NetDeviceContainer Install(Ptr<Node> a, Ptr<Node> b);

    NetDeviceContainer Install(Ptr<Node> a, std::string bName);

    NetDeviceContainer Install(std::string aName, Ptr<Node> b);

    NetDeviceContainer Install(std::string aNode, std::string bNode);

  private:
    void EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd,
                            bool promiscuous, bool explicitFilename) override;

    void EnableAsciiInternal(Ptr<OutputStreamWrapper> stream,
                             std::string prefix, Ptr<NetDevice> nd,
                             bool explicitFilename) override;

    ObjectFactory m_queueFactory;
    ObjectFactory m_channelFactory;
    ObjectFactory m_deviceFactory;
    bool m_enableFlowControl;
};

/***************************************************************
 *  Implementation of the templates declared above.
 ***************************************************************/

template <typename FRR_POLICY>
template <typename... Ts>
void PointToPointFRRHelper<FRR_POLICY>::SetQueue(std::string type, Ts&&... args)
{
    QueueBase::AppendItemTypeIfNotPresent(type, "Packet");

    m_queueFactory.SetTypeId(type);
    m_queueFactory.Set(std::forward<Ts>(args)...);
}

template <typename FRR_POLICY>
PointToPointFRRHelper<FRR_POLICY>::PointToPointFRRHelper()
{
    m_queueFactory.SetTypeId("ns3::DropTailQueue<Packet>");
    m_deviceFactory.SetTypeId(
        ns3::PointToPointFRRNetDevice<FRR_POLICY>::getNetDeviceString());
    m_channelFactory.SetTypeId(
        ns3::PointToPointFRRChannel<FRR_POLICY>::getChannelString());
    m_enableFlowControl = true;
}

template <typename FRR_POLICY>
void PointToPointFRRHelper<FRR_POLICY>::SetDeviceAttribute(
    std::string n1, const AttributeValue& v1)
{
    m_deviceFactory.Set(n1, v1);
}

template <typename FRR_POLICY>
void PointToPointFRRHelper<FRR_POLICY>::SetChannelAttribute(
    std::string n1, const AttributeValue& v1)
{
    m_channelFactory.Set(n1, v1);
}

template <typename FRR_POLICY>
void PointToPointFRRHelper<FRR_POLICY>::DisableFlowControl()
{
    m_enableFlowControl = false;
}

template <typename FRR_POLICY>
void PointToPointFRRHelper<FRR_POLICY>::EnablePcapInternal(
    std::string prefix, Ptr<NetDevice> nd, bool promiscuous,
    bool explicitFilename)
{
    //
    // All of the Pcap enable functions vector through here including the ones
    // that are wandering through all of devices on perhaps all of the nodes in
    // the system.  We can only deal with devices of type
    // PointToPointFRRNetDevice.
    //
    Ptr<PointToPointFRRNetDevice<FRR_POLICY>> device =
        nd->GetObject<PointToPointFRRNetDevice<FRR_POLICY>>();
    if (!device) {
        NS_LOG_INFO("Device " << device
                              << " not of type ns3::PointToPointFRRNetDevice");
        return;
    }

    PcapHelper pcapHelper;

    std::string filename;
    if (explicitFilename) {
        filename = prefix;
    } else {
        filename = pcapHelper.GetFilenameFromDevice(prefix, device);
    }

    Ptr<PcapFileWrapper> file =
        pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_PPP);
    pcapHelper.HookDefaultSink<PointToPointFRRNetDevice<FRR_POLICY>>(
        device, "PromiscSniffer", file);
}

template <typename FRR_POLICY>
void PointToPointFRRHelper<FRR_POLICY>::EnableAsciiInternal(
    Ptr<OutputStreamWrapper> stream, std::string prefix, Ptr<NetDevice> nd,
    bool explicitFilename)
{
    //
    // All of the ascii enable functions vector through here including the ones
    // that are wandering through all of devices on perhaps all of the nodes in
    // the system.  We can only deal with devices of type
    // PointToPointFRRNetDevice.
    //
    Ptr<PointToPointFRRNetDevice<FRR_POLICY>> device =
        nd->GetObject<PointToPointFRRNetDevice<FRR_POLICY>>();
    if (!device) {
        NS_LOG_INFO("Device " << device
                              << " not of type ns3::PointToPointFRRNetDevice");
        return;
    }

    //
    // Our default trace sinks are going to use packet printing, so we have to
    // make sure that is turned on.
    //
    Packet::EnablePrinting();

    //
    // If we are not provided an OutputStreamWrapper, we are expected to create
    // one using the usual trace filename conventions and do a
    // Hook*WithoutContext since there will be one file per context and
    // therefore the context would be redundant.
    //
    if (!stream) {
        //
        // Set up an output stream object to deal with private ofstream copy
        // constructor and lifetime issues.  Let the helper decide the actual
        // name of the file given the prefix.
        //
        AsciiTraceHelper asciiTraceHelper;

        std::string filename;
        if (explicitFilename) {
            filename = prefix;
        } else {
            filename = asciiTraceHelper.GetFilenameFromDevice(prefix, device);
        }

        Ptr<OutputStreamWrapper> theStream =
            asciiTraceHelper.CreateFileStream(filename);

        //
        // The MacRx trace source provides our "r" event.
        //
        asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<
            PointToPointFRRNetDevice<FRR_POLICY>>(device, "MacRx", theStream);

        //
        // The "+", '-', and 'd' events are driven by trace sources actually in
        // the transmit queue.
        //
        Ptr<Queue<Packet>> queue = device->GetQueue();
        asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<Queue<Packet>>(
            queue, "Enqueue", theStream);
        asciiTraceHelper.HookDefaultDropSinkWithoutContext<Queue<Packet>>(
            queue, "Drop", theStream);
        asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<Queue<Packet>>(
            queue, "Dequeue", theStream);

        // PhyRxDrop trace source for "d" event
        asciiTraceHelper.HookDefaultDropSinkWithoutContext<
            PointToPointFRRNetDevice<FRR_POLICY>>(device, "PhyRxDrop",
                                                  theStream);

        return;
    }

    //
    // If we are provided an OutputStreamWrapper, we are expected to use it, and
    // to providd a context.  We are free to come up with our own context if we
    // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
    // compatibility and simplicity, we just use Config::Connect and let it deal
    // with the context.
    //
    // Note that we are going to use the default trace sinks provided by the
    // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
    // but the default trace sinks are actually publicly available static
    // functions that are always there waiting for just such a case.
    //
    uint32_t nodeid = nd->GetNode()->GetId();
    uint32_t deviceid = nd->GetIfIndex();
    std::ostringstream oss;

    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::PointToPointFRRNetDevice/MacRx";
    Config::Connect(
        oss.str(),
        MakeBoundCallback(&AsciiTraceHelper::DefaultReceiveSinkWithContext,
                          stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::PointToPointFRRNetDevice/TxQueue/Enqueue";
    Config::Connect(
        oss.str(),
        MakeBoundCallback(&AsciiTraceHelper::DefaultEnqueueSinkWithContext,
                          stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::PointToPointFRRNetDevice/TxQueue/Dequeue";
    Config::Connect(
        oss.str(),
        MakeBoundCallback(&AsciiTraceHelper::DefaultDequeueSinkWithContext,
                          stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::PointToPointFRRNetDevice/TxQueue/Drop";
    Config::Connect(oss.str(),
                    MakeBoundCallback(
                        &AsciiTraceHelper::DefaultDropSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::PointToPointFRRNetDevice/PhyRxDrop";
    Config::Connect(oss.str(),
                    MakeBoundCallback(
                        &AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}

template <typename FRR_POLICY>
NetDeviceContainer PointToPointFRRHelper<FRR_POLICY>::Install(NodeContainer c)
{
    NS_ASSERT(c.GetN() == 2);
    return Install(c.Get(0), c.Get(1));
}

template <typename FRR_POLICY>
NetDeviceContainer PointToPointFRRHelper<FRR_POLICY>::Install(Ptr<Node> a,
                                                              Ptr<Node> b)
{
    NetDeviceContainer container;

    Ptr<PointToPointFRRNetDevice<FRR_POLICY>> devA =
        m_deviceFactory.Create<PointToPointFRRNetDevice<FRR_POLICY>>();
    devA->SetAddress(Mac48Address::Allocate());
    a->AddDevice(devA);
    Ptr<Queue<Packet>> queueA = m_queueFactory.Create<Queue<Packet>>();
    devA->SetQueue(queueA);
    Ptr<PointToPointFRRNetDevice<FRR_POLICY>> devB =
        m_deviceFactory.Create<PointToPointFRRNetDevice<FRR_POLICY>>();
    devB->SetAddress(Mac48Address::Allocate());
    b->AddDevice(devB);
    Ptr<Queue<Packet>> queueB = m_queueFactory.Create<Queue<Packet>>();
    devB->SetQueue(queueB);
    if (m_enableFlowControl) {
        // Aggregate NetDeviceQueueInterface objects
        Ptr<NetDeviceQueueInterface> ndqiA =
            CreateObject<NetDeviceQueueInterface>();
        ndqiA->GetTxQueue(0)->ConnectQueueTraces(queueA);
        devA->AggregateObject(ndqiA);
        Ptr<NetDeviceQueueInterface> ndqiB =
            CreateObject<NetDeviceQueueInterface>();
        ndqiB->GetTxQueue(0)->ConnectQueueTraces(queueB);
        devB->AggregateObject(ndqiB);
    }

    Ptr<PointToPointFRRChannel<FRR_POLICY>> channel = nullptr;

    // If MPI is enabled, we need to see if both nodes have the same system id
    // (rank), and the rank is the same as this instance.  If both are true,
    // use a normal p2p channel, otherwise use a remote channel
#ifdef NS3_MPI
    bool useNormalChannel = true;
    if (MpiInterface::IsEnabled()) {
        uint32_t n1SystemId = a->GetSystemId();
        uint32_t n2SystemId = b->GetSystemId();
        uint32_t currSystemId = MpiInterface::GetSystemId();
        if (n1SystemId != currSystemId || n2SystemId != currSystemId) {
            useNormalChannel = false;
        }
    }
    if (useNormalChannel) {
        m_channelFactory.SetTypeId("ns3::PointToPointFRRChannel");
        channel = m_channelFactory.Create<PointToPointFRRChannel<FRR_POLICY>>();
    } else {
        m_channelFactory.SetTypeId("ns3::PointToPointRemoteChannel");
        channel = m_channelFactory.Create<PointToPointRemoteChannel>();
        Ptr<MpiReceiver> mpiRecA = CreateObject<MpiReceiver>();
        Ptr<MpiReceiver> mpiRecB = CreateObject<MpiReceiver>();
        mpiRecA->SetReceiveCallback(
            MakeCallback(&PointToPointFRRNetDevice<FRR_POLICY>::Receive, devA));
        mpiRecB->SetReceiveCallback(
            MakeCallback(&PointToPointFRRNetDevice<FRR_POLICY>::Receive, devB));
        devA->AggregateObject(mpiRecA);
        devB->AggregateObject(mpiRecB);
    }
#else
    channel = m_channelFactory.Create<PointToPointFRRChannel<FRR_POLICY>>();
#endif

    devA->Attach(channel);
    devB->Attach(channel);
    container.Add(devA);
    container.Add(devB);

    return container;
}

template <typename FRR_POLICY>
NetDeviceContainer PointToPointFRRHelper<FRR_POLICY>::Install(Ptr<Node> a,
                                                              std::string bName)
{
    Ptr<Node> b = Names::Find<Node>(bName);
    return Install(a, b);
}

template <typename FRR_POLICY>
NetDeviceContainer PointToPointFRRHelper<FRR_POLICY>::Install(std::string aName,
                                                              Ptr<Node> b)
{
    Ptr<Node> a = Names::Find<Node>(aName);
    return Install(a, b);
}

template <typename FRR_POLICY>
NetDeviceContainer PointToPointFRRHelper<FRR_POLICY>::Install(std::string aName,
                                                              std::string bName)
{
    Ptr<Node> a = Names::Find<Node>(aName);
    Ptr<Node> b = Names::Find<Node>(bName);
    return Install(a, b);
}

} // namespace ns3

#endif
