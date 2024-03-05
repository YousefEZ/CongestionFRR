#include <iostream>

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

#include "../libs/dummy_congestion_policy.h"
#include "../libs/frr_queue.h"
#include "../libs/lfa_policy.h"
#include "../libs/modulo_congestion_policy.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CongestionFastReRoute");

// place the policies for FRR here
using CongestionPolicy = ModuloCongestionPolicy<2>;

using FRRPolicy = LFAPolicy;

using SimulationQueue = FRRQueue<CongestionPolicy, FRRPolicy>;

NS_OBJECT_ENSURE_REGISTERED(SimulationQueue);

template <int INDEX>
Ptr<PointToPointNetDevice> getDevice(const NetDeviceContainer &devices) {
  return devices.Get(INDEX)->GetObject<PointToPointNetDevice>();
}

template <int INDEX>
Ptr<SimulationQueue> getQueue(const NetDeviceContainer &devices) {
  return DynamicCast<SimulationQueue>(getDevice<INDEX>(devices)->GetQueue());
}

template <int INDEX>
void setAlternateTarget(const NetDeviceContainer &devices,
                        Ptr<PointToPointNetDevice> target) {
  getQueue<INDEX>(devices)->addAlternateTargets(target);
}

// Location to store traces
std::string tracesPath = "traces/";

// ---[TCP PARAMETERS] ---
uint32_t segmentSize = 1024;
uint32_t MTU_bytes = segmentSize + 54;
uint8_t delAckCount = 1;
uint8_t initialCwnd = 1;
std::string delAckTimeout = "1ms";
std::string socketFactory = "ns3::TcpSocketFactory";
std::string qdiscTypeId = "ns3::PFifoFastQueueDisc";
std::string tcpRecovery = "ns3::TcpClassicRecovery";
std::string tcpVariantId = "ns3::TcpLinuxReno";
bool enableSack = false;
double minRTO = 1.0;

void TraceCwnd(uint32_t node, uint32_t cwndWindow,
               Callback<void, uint32_t, uint32_t> CwndTrace) {
  Config::ConnectWithoutContext(
      "/NodeList/" + std::to_string(node) + "/$ns3::TcpL4Protocol/SocketList/" +
          std::to_string(cwndWindow) + "/CongestionWindow",
      CwndTrace);
}

static void CwndChange(uint32_t oldCwnd, uint32_t newCwnd) {
  std::ofstream fPlotQueue(tracesPath + "cwnd.txt",
                           std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now().GetSeconds() << " " << newCwnd / segmentSize
             << " " << newCwnd << std::endl;
  fPlotQueue.close();
}

void InstallBulkSend(Ptr<Node> node, Ipv4Address address, uint16_t port,
                     std::string socketFactory, uint32_t nodeId,
                     uint32_t cwndWindow,
                     Callback<void, uint32_t, uint32_t> CwndTrace,
                     uint32_t maxBytesToSend, Time startTime) {
  BulkSendHelper source(socketFactory, InetSocketAddress(address, port));
  source.SetAttribute("MaxBytes", UintegerValue(maxBytesToSend));
  ApplicationContainer sourceApps = source.Install(node);
  sourceApps.Start(startTime);
  sourceApps.Stop(Seconds(5));
  // Simulator::Schedule(startTime + Seconds(0.001), nullptr, nodeId,
  // cwndWindow,
  //                  nullptr);
}

int main(int argc, char *argv[]) {
  // TCP Recovery Algorithm
  Config::SetDefault("ns3::TcpL4Protocol::RecoveryType",
                     TypeIdValue(TypeId::LookupByName(tcpRecovery)));

  // Set Congestion Control Algorithm
  Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                     StringValue(tcpVariantId));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1073741824));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1073741824));

  // Set default initial congestion window
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(initialCwnd));

  // Set default delayed ack count to a specified value
  Config::SetDefault("ns3::TcpSocket::DelAckTimeout",
                     TimeValue(Time(delAckTimeout)));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(delAckCount));
  // Config::SetDefault("ns3::TcpSocket::SetTcpNoDelay", BooleanValue(true));

  // Set default segment size of TCP packet to a specified value
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(segmentSize));

  // Enable/Disable SACK in TCP
  Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(enableSack));

  Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(Seconds(minRTO)));

  LogComponentEnable("CongestionFastReRoute", LOG_LEVEL_INFO);
  NodeContainer nodes;
  nodes.Create(3);
  Names::Add("Node0", nodes.Get(0));

  InternetStackHelper stack;
  stack.Install(nodes);

  // Configure PointToPoint links
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("1ms"));

  // Set the custom queue for the device
  p2p.SetQueue(SimulationQueue::getQueueString());

  // Install devices and channels between nodes
  NetDeviceContainer devices01 = p2p.Install(nodes.Get(0), nodes.Get(1));
  NetDeviceContainer devices12 = p2p.Install(nodes.Get(1), nodes.Get(2));
  // Add the missing link between Node 0 and Node 2 to fully connect the network
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

  // Config::SetDefault("ns3::PfifoFastQueueDisc::MaxSize",
  //                  QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, 1)));
  // Config::SetDefault("ns3::DropTailQueue<Packet>::MaxSize",
  // StringValue("1p"));
  // Config::SetDefault(SimulationQueue::getQueueString() + "::MaxSize",
  //                 StringValue("1p"));

  // TrafficControlHelper tch;
  // tch.SetRootQueueDisc("ns3::PfifoFastQueueDisc");
  // QueueDiscContainer qd;
  // tch.Uninstall(nodes.Get(0)->GetDevice(0));
  // qd.Add(tch.Install(nodes.Get(0)->GetDevice(0)).Get(0));
  // tch.Uninstall(nodes.Get(1)->GetDevice(0));
  // qd.Add(tch.Install(nodes.Get(1)->GetDevice(0)).Get(0));
  // tch.Uninstall(nodes.Get(2)->GetDevice(0));
  // qd.Add(tch.Install(nodes.Get(2)->GetDevice(0)).Get(0));

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

  // ApplicationContainer app = onoff.Install(nodes.Get(0));
  // app.Start(Seconds(1.0));
  // app.Stop(Seconds(10.0));
  // Set up an alternate forwarding target, assuming you have an alternate path
  // configured

  setAlternateTarget<0>(devices01, getDevice<0>(devices02));
  setAlternateTarget<0>(devices02, getDevice<0>(devices01));

  setAlternateTarget<0>(devices12, getDevice<1>(devices01));
  setAlternateTarget<1>(devices01, getDevice<0>(devices12));

  setAlternateTarget<1>(devices02, getDevice<1>(devices12));
  setAlternateTarget<1>(devices12, getDevice<1>(devices02));

  p2p.EnablePcapAll("traces/");

  /* Install BulkSend application */
  InstallBulkSend(nodes.Get(0), interfaces01.GetAddress(1), 5000,
                  "ns3::TcpSocketFactory", 2, 0, nullptr, 1000, Seconds(0.2));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
