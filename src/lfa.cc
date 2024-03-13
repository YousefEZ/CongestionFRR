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
#include "../libs/point_to_point/point_to_point_frr_helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CongestionFastReRoute");

// place the policies for FRR here
using CongestionPolicy = ModuloCongestionPolicy<2>;

using FRRPolicy = LFAPolicy;

using SimulationQueue = FRRQueue<CongestionPolicy, FRRPolicy>;

NS_OBJECT_ENSURE_REGISTERED(SimulationQueue);

template <int INDEX>
Ptr<PointToPointFRRNetDevice> getDevice(const NetDeviceContainer& devices)
{
    return devices.Get(INDEX)->GetObject<PointToPointFRRNetDevice>();
}

template <int INDEX>
Ptr<SimulationQueue> getQueue(const NetDeviceContainer& devices)
{
    return DynamicCast<SimulationQueue>(getDevice<INDEX>(devices)->GetQueue());
}

template <int INDEX>
void setAlternateTarget(const NetDeviceContainer& devices,
                        Ptr<PointToPointFRRNetDevice> target)
{
    getQueue<INDEX>(devices)->addAlternateTargets(target);
}

int main(int argc, char* argv[])
{
    LogComponentEnable("CongestionFastReRoute", LOG_LEVEL_INFO);
    NodeContainer nodes;
    nodes.Create(3);

    InternetStackHelper stack;
    stack.Install(nodes);

    // Configure PointToPoint links
    PointToPointFRRHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));

    // Set the custom queue for the device
    p2p.SetQueue(SimulationQueue::getQueueString());

    // Install devices and channels between nodes
    NetDeviceContainer devices01 = p2p.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer devices12 = p2p.Install(nodes.Get(1), nodes.Get(2));
    // Add the missing link between Node 0 and Node 2 to fully connect the
    // network
    NetDeviceContainer devices02 = p2p.Install(nodes.Get(0), nodes.Get(2));

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces02 = address.Assign(devices02);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Configure the application to generate traffic
    // we have node 0 sending traffic to node 2
    //
    //     1
    //    / \
    //   /   \
    //  /     \
    // 0 -----> 2
    //
    uint16_t port = 9;
    OnOffHelper onoff("ns3::UdpSocketFactory",
                      InetSocketAddress(interfaces12.GetAddress(1), port));
    onoff.SetAttribute("OnTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute("OffTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onoff.SetAttribute("DataRate", DataRateValue(DataRate("1Mbps")));
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

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
