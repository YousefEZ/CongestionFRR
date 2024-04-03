#include <iostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h"
#include "ns3/traffic-control-module.h"

#include "../libs/frr_queue.h"
#include "../libs/dummy_congestion_policy.h"
#include "../libs/modulo_congestion_policy.h"
#include "../libs/lfa_policy.h"
#include "../libs/random_congestion_policy.h"
#include "../libs/point_to_point_frr_helper.h"
#include "../libs/basic_congestion.h"

using namespace ns3;

using CongestionPolicy = BasicCongestionPolicy<99>;
// using CongestionPolicy = RandomCongestionPolicy<100>;
using FRRPolicy = LFAPolicy;

using SimulationQueue = FRRQueue<CongestionPolicy>;
using FRRNetDevice = PointToPointFRRNetDevice<FRRPolicy>;
using FRRChannel = PointToPointFRRChannel<FRRPolicy>;

void toggleCongestion(Ptr<SimulationQueue> queue)
{
    ;
    // queue->m_congestionPolicy.turnOff();
}

// void enableRerouting(Ptr<SimulationQueue> queue)
// {
//     // queue->m_congestionPolicy.enable();
// }

NS_OBJECT_ENSURE_REGISTERED(SimulationQueue);
NS_OBJECT_ENSURE_REGISTERED(FRRChannel);
NS_OBJECT_ENSURE_REGISTERED(FRRNetDevice);

template <int INDEX, typename DEVICE_TYPE>
Ptr<DEVICE_TYPE> getDevice(const NetDeviceContainer& devices)
{
    return devices.Get(INDEX)->GetObject<DEVICE_TYPE>();
}

template <int INDEX>
Ptr<SimulationQueue> getQueue(const NetDeviceContainer& devices)
{
    return DynamicCast<SimulationQueue>(
        getDevice<INDEX, FRRNetDevice>(devices)->GetQueue());
}

template <int INDEX>
void setAlternateTarget(const NetDeviceContainer& devices,
                        Ptr<ns3::NetDevice> target)
{
    getDevice<INDEX, FRRNetDevice>(devices)->addAlternateTarget(target);
}

// TCP parameters
uint32_t segmentSize = 1024;
uint32_t MTU_bytes = segmentSize + 54;

// Topology parameters
std::string bandwidth_bottleneck = "150Kbps";
std::string bandwidth_access = "600kbps";
std::string bandwidth_udp_access = "100kbps";
std::string delay_bottleneck = "20ms";
std::string delay_access = "20ms";
std::string delay_alternate = "20ms";
std::string bandwidth_alternate = "600kbps";

void SetupTCPConfig()
{
    // TCP recovery algorithm
    Config::SetDefault(
        "ns3::TcpL4Protocol::RecoveryType",
        TypeIdValue(TypeId::LookupByName("ns3::TcpClassicRecovery")));
    // Congestion control algorithm
    Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                       StringValue("ns3::TcpLinuxReno"));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1073741824));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1073741824));
    // Initial congestion window
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));
    // Set delayed ack count
    Config::SetDefault("ns3::TcpSocket::DelAckTimeout", TimeValue(Time("1ms")));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));
    // Set segment size of packet
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1024));
    // Enable/disable SACKs (disabled)
    Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(false));
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(Seconds(1.0)));
}

void CalculateExpectedPackets(uint32_t tcp_max_bytes, DataRate udp_data_rate)
{
    DataRate bandwidth_bottleneck_dr(bandwidth_bottleneck);
    DataRate bandwidth_access_dr(bandwidth_access);
    Time bottleneck_delay_t(delay_bottleneck);
    Time access_delay_t(delay_access);

    // Serialization delay ~2ms
    Time serialization_delay_t(
        (1024 + 54) /
        (std::min(bandwidth_bottleneck_dr, bandwidth_access_dr).GetBitRate()));

    uint32_t max_bottleneck_tcp_bytes = static_cast<uint32_t>(
        ((std::min(bandwidth_access_dr, bandwidth_bottleneck_dr).GetBitRate() /
          8) *
         (((access_delay_t * 2) + bottleneck_delay_t) * 2 +
          serialization_delay_t)
             .GetSeconds()));

    uint32_t expected_tcp_packets =
        std::ceil(max_bottleneck_tcp_bytes / (1024 + 54));
    std::cout << "Expected TCP packets in queue: " << expected_tcp_packets
              << std::endl;
}

// NS_LOG_COMPONENT_DEFINE("CongestionFastReRoute");
int main(int argc, char* argv[])
{
    LogComponentEnable("FRRQueue", LOG_LEVEL_LOGIC);
    /*
     *  +----------+      +-----------+
     *  |Congestion|      |  Traffic  |
     *  |  Sender  |      |  Sender   |
     * 0+----+-----+     1+-----+-----+
     *       |                  |
     *       |   +----------+   |
     *       +---+  Router  +---+
     *           |    01    |
     *          2+----+-----+--------+
     *                |              |
     *                |        +-----+----+
     *                |        |  Router  |
     *                |        |    03    |
     *           +----+-----+ 4+----+-----+
     *           |  Router  |       |
     *           |    02    +-------+
     *          3+----+-----+
     *                |
     *                |
     *           +----+-----+
     *           | Receiver |
     *           |          |
     *          5+----------+
     */
    // Topology setup
    NS_LOG_INFO("Creating Topology");
    NodeContainer nodes;
    nodes.Create(6);
    Names::Add("CongestionSender", nodes.Get(0));
    Names::Add("TrafficSender", nodes.Get(1));
    Names::Add("Router01", nodes.Get(2));
    Names::Add("Router02", nodes.Get(3));
    Names::Add("Router03", nodes.Get(4));
    Names::Add("Receiver", nodes.Get(5));

    InternetStackHelper stack;
    stack.Install(nodes);

    // Configure PointToPoint link for normal traffic
    PointToPointHelper p2p_traffic;
    p2p_traffic.SetDeviceAttribute("DataRate", StringValue(bandwidth_access));
    p2p_traffic.SetChannelAttribute("Delay", StringValue(delay_access));
    // Set the custom queue for the device
    p2p_traffic.SetQueue("ns3::DropTailQueue<Packet>");
    // Install devices and channels between nodes

    // PointToPointFRRHelper<FRRPolicy> p2p_congested_link;
    PointToPointHelper p2p_congested_link;
    p2p_congested_link.SetDeviceAttribute("DataRate",
                                          StringValue(bandwidth_bottleneck));
    p2p_congested_link.SetChannelAttribute("Delay",
                                           StringValue(delay_bottleneck));
    // p2p_congested_link.SetQueue(SimulationQueue::getQueueString());
    p2p_congested_link.SetQueue("ns3::DropTailQueue<Packet>");

    Config::SetDefault("ns3::DropTailQueue<Packet>::MaxSize",
                       StringValue("10p"));
    Config::SetDefault(SimulationQueue::getQueueString() + "::MaxSize",
                       StringValue("10p"));

    PointToPointHelper p2p_alternate;
    p2p_alternate.SetDeviceAttribute("DataRate",
                                     StringValue(bandwidth_alternate));
    p2p_alternate.SetChannelAttribute("Delay", StringValue(delay_alternate));
    p2p_alternate.SetQueue("ns3::DropTailQueue<Packet>");

    NetDeviceContainer devices_1_2 =
        p2p_traffic.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer devices_2_3 =
        p2p_congested_link.Install(nodes.Get(2), nodes.Get(3));
    NetDeviceContainer devices_2_4 =
        p2p_alternate.Install(nodes.Get(2), nodes.Get(4));
    NetDeviceContainer devices_4_3 =
        p2p_alternate.Install(nodes.Get(4), nodes.Get(3));
    NetDeviceContainer devices_3_5 =
        p2p_traffic.Install(nodes.Get(3), nodes.Get(5));

    // Configure PointToPoint link for congestion link
    PointToPointHelper p2p_congestion;
    p2p_congestion.SetDeviceAttribute("DataRate",
                                      StringValue(bandwidth_udp_access));
    p2p_congestion.SetChannelAttribute("Delay", StringValue(delay_access));
    // Set the custom queue for the device
    p2p_congestion.SetQueue("ns3::DropTailQueue<Packet>");
    // Install devices and channels between nodes
    NetDeviceContainer devices_0_2 =
        p2p_congestion.Install(nodes.Get(0), nodes.Get(2));

    // Assign IP addresses to subnets
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_0_2 = address.Assign(devices_0_2);
    Ipv4InterfaceContainer interfaces_1_2 = address.Assign(devices_1_2);
    address.NewNetwork();
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_2_3 = address.Assign(devices_2_3);
    address.NewNetwork();
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_2_4 = address.Assign(devices_2_4);
    address.NewNetwork();
    address.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_4_3 = address.Assign(devices_4_3);
    address.NewNetwork();
    address.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_3_5 = address.Assign(devices_3_5);
    address.NewNetwork();
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Receiver address
    Ipv4Address receiver_addr = interfaces_3_5.GetAddress(1);

    // UDP Congestion traffic setup
    uint16_t udp_port = 50001;
    OnOffHelper udp_source("ns3::UdpSocketFactory",
                           InetSocketAddress(receiver_addr, udp_port));
    udp_source.SetAttribute(
        "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udp_source.SetAttribute(
        "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    udp_source.SetAttribute("DataRate",
                            DataRateValue(DataRate(bandwidth_udp_access)));
    udp_source.SetAttribute("PacketSize", UintegerValue(1024));

    // ApplicationContainer udp_app = udp_source.Install(nodes.Get(0));
    // udp_app.Start(Seconds(0.0));
    // udp_app.Stop(Seconds(5.0));

    DataRate b_access(bandwidth_access);
    DataRate b_bottleneck(bandwidth_bottleneck);
    Time d_access(delay_access);
    Time d_bottleneck(delay_bottleneck);
    Time d_serialization("1.9ms");

    // TCP Setup
    SetupTCPConfig();
    uint16_t tcp_port = 50002;
    BulkSendHelper tcp_source("ns3::TcpSocketFactory",
                              InetSocketAddress(receiver_addr, tcp_port));
    tcp_source.SetAttribute("MaxBytes",
                            UintegerValue(100000)); // 0 for unlimited data
    tcp_source.SetAttribute("SendSize",
                            UintegerValue(1024)); // Packet size in bytes
    ApplicationContainer tcp_app = tcp_source.Install(nodes.Get(1));
    tcp_app.Start(Seconds(0.0));
    tcp_app.Stop(Seconds(5.0));

    // Packet sink setup (Receiver node)
    PacketSinkHelper sink("ns3::TcpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), tcp_port));
    ApplicationContainer sink_app = sink.Install(nodes.Get(5));
    sink_app.Start(Seconds(0.0));
    sink_app.Stop(Seconds(10.0));

    PacketSinkHelper udp_sink(
        "ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), udp_port));
    ApplicationContainer udp_sink_app = udp_sink.Install(nodes.Get(5));
    udp_sink_app.Start(Seconds(0.0));
    udp_sink_app.Stop(Seconds(10.0));
    // SimulationQueue::sinkAddress =
    //     Mac48Address::ConvertFrom(getDevice<1>(devices_3_5)->GetAddress());
    // NOTE: Is TrafficControlHelper needed here?

    // CalculateExpectedPackets(10000, DataRate("1Mbps"));

    // LFA Alternate Path setup
    // Set up an alternate forwarding target, assuming you have an alternate
    // path configured

    // TODO: Need some help with setting alternate target
    // setAlternateTarget<0>(
    //     devices_2_3, getDevice<0, ns3::PointToPointNetDevice>(devices_2_4));
    // setAlternateTarget<1>(
    //     devices_2_3, getDevice<1, ns3::PointToPointNetDevice>(devices_4_3));
    // setAlternateTarget<0>(devices01, getDevice<0>(devices02));
    // setAlternateTarget<0>(devices02, getDevice<0>(devices01));

    // setAlternateTarget<0>(devices12, getDevice<1>(devices01));
    // setAlternateTarget<1>(devices01, getDevice<0>(devices12));

    // setAlternateTarget<1>(devices02, getDevice<1>(devices12));
    // setAlternateTarget<1>(devices12, getDevice<1>(devices02));

    // enableRerouting(getQueue<0>(devices_2_3));
    p2p_traffic.EnablePcapAll("traces/");
    p2p_congestion.EnablePcapAll("traces/");

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
