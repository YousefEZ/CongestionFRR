#include <iostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h"

#include "../libs/frr_queue.h"
#include "../libs/dummy_congestion_policy.h"
#include "../libs/modulo_congestion_policy.h"
#include "../libs/lfa_policy.h"
#include "../libs/random_congestion_policy.h"
#include "../libs/point_to_point_frr_helper.h"

using namespace ns3;

using CongestionPolicy = RandomCongestionPolicy<100>;
// using CongestionPolicy = BasicCongestionPolicy<50>;
using FRRPolicy = LFAPolicy;

using SimulationQueue = FRRQueue<CongestionPolicy>;
using FRRNetDevice = PointToPointFRRNetDevice<FRRPolicy>;
using FRRChannel = PointToPointFRRChannel<FRRPolicy>;

void toggleCongestion(Ptr<SimulationQueue> queue)
{
    queue->m_congestionPolicy.turnOff();
}

NS_OBJECT_ENSURE_REGISTERED(SimulationQueue);
NS_OBJECT_ENSURE_REGISTERED(FRRChannel);
NS_OBJECT_ENSURE_REGISTERED(FRRNetDevice);

template <int INDEX>
Ptr<FRRNetDevice> getDevice(const NetDeviceContainer& devices)
{
    return devices.Get(INDEX)->GetObject<FRRNetDevice>();
}

template <int INDEX>
Ptr<SimulationQueue> getQueue(const NetDeviceContainer& devices)
{
    return DynamicCast<SimulationQueue>(getDevice<INDEX>(devices)->GetQueue());
}

template <int INDEX>
void setAlternateTarget(const NetDeviceContainer& devices,
                        Ptr<FRRNetDevice> target)
{
    getDevice<INDEX>(devices)->addAlternateTarget(target);
}

// NS_LOG_COMPONENT_DEFINE("CongestionFastReRoute");

int main(int argc, char* argv[])
{
    LogComponentEnable("FRRQueue", LOG_LEVEL_LOGIC);
    NS_LOG_INFO("Creating Topology");
    NodeContainer nodes;
    nodes.Create(3);

    InternetStackHelper stack;
    stack.Install(nodes);

    // Configure PointToPoint links
    PointToPointFRRHelper<FRRPolicy> p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));

    // Set the custom queue for the device
    p2p.SetQueue(SimulationQueue::getQueueString());

    // Install devices and channels between nodes
    NetDeviceContainer devices01 = p2p.Install(nodes.Get(0), nodes.Get(1));
    std::cout << "0 -> 1: " << getQueue<0>(devices01)->m_uid << std::endl;
    std::cout << "1 -> 0: " << getQueue<1>(devices01)->m_uid << std::endl;
    NetDeviceContainer devices12 = p2p.Install(nodes.Get(1), nodes.Get(2));
    std::cout << "1 -> 2: " << getQueue<0>(devices12)->m_uid << std::endl;
    std::cout << "2 -> 1: " << getQueue<1>(devices12)->m_uid << std::endl;
    // Add the missing link between Node 0 and Node 2 to fully connect the
    // network
    NetDeviceContainer devices02 = p2p.Install(nodes.Get(0), nodes.Get(2));
    std::cout << "0 -> 2: " << getQueue<0>(devices02)->m_uid << std::endl;
    std::cout << "2 -> 0: " << getQueue<1>(devices02)->m_uid << std::endl;

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces02 = address.Assign(devices02);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    Ptr<OutputStreamWrapper> routingStream =
        Create<OutputStreamWrapper>(&std::cout);
    Ipv4GlobalRoutingHelper::PrintRoutingTableAllAt(Seconds(1), routingStream);
    /* Configure the application to generate traffic
     * we have node 0 sending traffic to node 2
     *
     *     1
     *    / \
     *   /   \
     *  /     \
     * 0 -----> 2
     */
    uint16_t port = 50000;
    OnOffHelper onoff("ns3::UdpSocketFactory",
                      InetSocketAddress(interfaces12.GetAddress(1), port));
    onoff.SetAttribute("OnTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute("OffTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onoff.SetAttribute("DataRate", DataRateValue(DataRate("100kbps")));
    onoff.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer app = onoff.Install(nodes.Get(0));
    app.Start(Seconds(1.0));
    app.Stop(Seconds(10.0));
    // Set up an alternate forwarding target, assuming you have an alternate
    // path configured

    setAlternateTarget<0>(devices01, getDevice<0>(devices02));
    setAlternateTarget<0>(devices02, getDevice<0>(devices01));

    setAlternateTarget<0>(devices12, getDevice<1>(devices01));
    setAlternateTarget<1>(devices01, getDevice<0>(devices12));

    setAlternateTarget<1>(devices02, getDevice<1>(devices12));
    setAlternateTarget<1>(devices12, getDevice<1>(devices02));

    // toggle off random congestion for all queues except node 0 to node 2
    toggleCongestion(getQueue<0>(devices01));
    toggleCongestion(getQueue<1>(devices01));
    toggleCongestion(getQueue<0>(devices12));
    toggleCongestion(getQueue<1>(devices12));
    toggleCongestion(getQueue<1>(devices02));

    p2p.EnablePcapAll("traces/");
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
