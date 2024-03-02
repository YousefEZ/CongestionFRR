// Filename: alternate-routing-example.cc
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "../libs/frr_queue.h"

using namespace ns3;


int main(int argc, char *argv[])
{
    LogComponentEnable("AlternateRoutingExample", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(3); 

    InternetStackHelper stack;
    stack.Install(nodes);

    // Configure PointToPoint links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));
    
    // Set the custom queue for the device
    p2p.SetQueue("ns3::FRRQueue<CongestionPolicy>");

    NetDeviceContainer devices01 = p2p.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer devices12 = p2p.Install(nodes.Get(1), nodes.Get(2));

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Configure the application to generate traffic
    uint16_t port = 9;
    OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(interfaces12.GetAddress(1), port)));
    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onoff.SetAttribute("DataRate", DataRateValue(DataRate("1Mbps")));
    onoff.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer app = onoff.Install(nodes.Get(0));
    app.Start(Seconds(1.0));
    app.Stop(Seconds(10.0));

    // Set up an alternate forwarding target, assuming you have an alternate path configured
    Ptr<FRRQueue<CongestionPolicy>> customQueue = DynamicCast<FRRQueue<CongestionPolicy>>(devices01.Get(0)->GetObject<PointToPointNetDevice>()->GetQueue());
    // customQueue->SetAlternateTarget(...); 

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

