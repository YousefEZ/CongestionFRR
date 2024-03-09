#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

#include "../libs/random/random.hpp"
#include "../libs/basic_congestion.h"

#include "../libs/frr_queue.h"
#include "../libs/lfa_policy.h"



using namespace ns3;
using Random = effolkronium::random_static;

using CongestionPolicy = BasicCongestionPolicy<99>;
using FRRPolicy = LFAPolicy;

using SimulationQueue = FRRQueue<CongestionPolicy, FRRPolicy>;

NS_OBJECT_ENSURE_REGISTERED(SimulationQueue);

// ---------------------------------------------------- //
// --- BEGIN OF SIMULATION CONFIGURATION PARAMETERS --- //
// ---------------------------------------------------- //

template <int INDEX>
Ptr<PointToPointNetDevice> getDevice(const NetDeviceContainer& devices) 
{
    return devices.Get(INDEX)->GetObject<PointToPointNetDevice>();
}

template <int INDEX>
Ptr<SimulationQueue> getQueue(const NetDeviceContainer& devices) 
{
    return DynamicCast<SimulationQueue>(getDevice<INDEX>(devices)->GetQueue());
}

template <int INDEX>
void setAlternateTarget(const NetDeviceContainer& devices, Ptr<PointToPointNetDevice> target) 
{
    getQueue<INDEX>(devices)->addAlternateTargets(target);
}


// Random Seed
uint32_t seed = 1;

// ---[TRACES]---
// Store Traces
bool storeTraces = true;

// Location to store traces
std::string tracesPath = "traces/";

// ---[SIMULATION PARAMETERS]---
// Time to initiate the TCP connection
Time startTimeTCP = Seconds(0.1);

// Time to stop the TCP connection
Time stopTimeTCP = Seconds(5);

// Simulation stop time
Time stopTimeSimulation = Seconds(5000);

// TCP flow interval
Time intervalTCP = Seconds(1);

// ---[TCP PARAMETERS] ---
uint32_t segmentSize = 32; //1024;
uint32_t MTU_bytes = 38;// segmentSize + 54;
uint8_t delAckCount = 1;
uint8_t initialCwnd = 1;
// std::string delAckTimeout = "200ms";
std::string delAckTimeout = "1ms";
std::string socketFactory = "ns3::TcpSocketFactory";
std::string qdiscTypeId = "ns3::PFifoFastQueueDisc";
std::string tcpRecovery = "ns3::TcpClassicRecovery";
// std::string tcpVariantId = "ns3::TcpCubic";
std::string tcpVariantId = "ns3::TcpLinuxReno";
bool enableSack = false;
double minRTO = 1.0;

// ---[TOPOLOGY PARAMETERS]---
std::string bandwidth_bottleneck = "1Mbps";
std::string bandwidth_access = "20Mbps";
std::string delay_access = "15ms";
std::string delay_bottleneck = "20ms";
std::string delay_serialization = "1.9ms";

// ---[POINTER TO THE DEVICE THAT WILL IMPLEMENT PACKET DROPING]
NetDeviceContainer *netDeviceToDropPacket = NULL;

// -------------------------------------------------- //
// --- END OF SIMULATION CONFIGURATION PARAMETERS --- //
// -------------------------------------------------- //

// Global Variables
Ptr<PacketSink> sinker;

int packetsDroppedInQueue = 0;
int64_t lastTotalRx = 0;
uint32_t bytes_to_send = 10000;

uint32_t cnt_packets = 0;

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
  sourceApps.Stop(stopTimeTCP);
  if (storeTraces == false) {
    Simulator::Schedule(startTime + Seconds(0.001), &TraceCwnd, nodeId,
                        cwndWindow, CwndTrace);
  }
}

void InstallPacketSink(Ptr<Node> node, uint16_t port, std::string socketFactory,
                       Time startTime, Time stopTime) {
  PacketSinkHelper sink(socketFactory,
                        InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer sinkApps = sink.Install(node);
  sinker = StaticCast<PacketSink>(sinkApps.Get(0));
  sinkApps.Start(startTime);
  sinkApps.Stop(stopTime);
}

static void DropAtQueue(Ptr<OutputStreamWrapper> stream,
                        Ptr<const QueueDiscItem> item) {
  *stream->GetStream() << Simulator::Now().GetSeconds() << " 1" << std::endl;
  packetsDroppedInQueue++;
}

void PacketsInQueueDisc(uint32_t oldValue, uint32_t newValue) {
  std::ofstream fPlotQueue(
      std::stringstream(tracesPath + "pktsQueueDisc.txt").str().c_str(),
      std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now().GetSeconds() << " " << newValue << std::endl;
  fPlotQueue.close();
}

void PacketsInDroptail(uint32_t oldValue, uint32_t newValue) {
  std::ofstream fPlotQueue(
      std::stringstream(tracesPath + "pktsDropTail.txt").str().c_str(),
      std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now().GetSeconds() << " " << newValue << std::endl;
  fPlotQueue.close();
}

void ExaminePacket(Ptr<const Packet> packet) {
  // Extract TCP Header from the packet
  TcpHeader tcpHeader;
  packet->PeekHeader(tcpHeader);
  uint32_t payloadSize = packet->GetSize();

  // Extract the SEQ and ACK numbers
  uint32_t seq = tcpHeader.GetSequenceNumber().GetValue();
  uint32_t ack = tcpHeader.GetAckNumber().GetValue();

  std::cout << "[TCP PACKET] [SEQ: " << seq << "] [ACK: " << ack
            << "] [Payload Length: " << payloadSize
            << "] PacketUid: " << packet->GetUid() << std::endl;
}

// void SimulateCongestion(Ptr<Node> congestionNode, Time startDelay,
//                         Time duration, unsigned int numPackets,
//                         unsigned int packetSize, Time interval,
//                         Ipv4Address receiverAddress, uint16_t port) {
//   BulkSendHelper source("ns3::UdpSocketFactory",
//                         InetSocketAddress(receiverAddress, port));
//
//   // Set bulk sender attributes
//   source.SetAttribute("MaxBytes", UintegerValue(numPackets * packetSize));
//   source.SetAttribute("SendSize", UintegerValue(packetSize));
//   // source.SetAttribute("interval", TimeValue(interval));
//
//   // Install application on congestionNode
//   ApplicationContainer apps = source.Install(congestionNode);
//   // And start it after specified delay
//   apps.Start(startDelay);
//   apps.Stop(startDelay + duration);
// }

void SimulateCongestion(Ptr<Node> congestionNode, Time startDelay,
                        Time duration, unsigned int numPackets,
                        unsigned int packetSize, Time interval,
                        Ipv4Address receiverAddress, uint16_t port) {
  // Create a UDP socket
  Ptr<Socket> socket =
      Socket::CreateSocket(congestionNode, UdpSocketFactory::GetTypeId());
  InetSocketAddress receiverSocketAddress(receiverAddress, port);

  // Schedule sending packets using the UDP socket
  Simulator::Schedule(startDelay, [socket, receiverSocketAddress, numPackets,
                                   packetSize, interval]() {
    for (unsigned int i = 0; i < numPackets; ++i) {
      // std::cout << "sending packet " << i << std::endl;
      Ptr<Packet> packet = Create<Packet>(packetSize);
      int err = socket->SendTo(packet, packetSize, receiverSocketAddress);
      if (err == -1) {
        std::cout << "error sending packet " << i << std::endl;
      }
    }
  });

  // Stop sending packets after the specified duration
  Simulator::Schedule(startDelay + duration, &Socket::Close, socket);
}

int main(int argc, char *argv[]) {
  LogComponentEnable("FRRQueue", LOG_LEVEL_LOGIC);
  NS_LOG_INFO("Creating Topology");
  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("tcpVariantId", "TCP variant", tcpVariantId);
  cmd.AddValue("enableSack", "Enable/disable sack in TCP", enableSack);
  cmd.AddValue("seed", "The random seed", seed);
  cmd.AddValue("simStopTime", "The simulation stop time", stopTimeSimulation);
  cmd.AddValue("intervalTCP", "The TCP interval", intervalTCP);
  cmd.AddValue("initialCwnd", "Initial CWND window", initialCwnd);
  cmd.AddValue("bytesToSend", "Number of bytes to send", bytes_to_send);
  cmd.Parse(argc, argv);

  // Set Random Seed
  Random::seed(seed);

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

  NodeContainer leftNodes, rightNodes, routers;
  routers.Create(3);
  leftNodes.Create(1);
  rightNodes.Create(1);

  Names::Add("Router1", routers.Get(0));
  Names::Add("Router2", routers.Get(1));
  Names::Add("Router3", routers.Get(2));
  Names::Add("Sender", leftNodes.Get(0));
  Names::Add("Receiver", rightNodes.Get(0));

  // Congestion node setup
  NodeContainer congestionNode;
  congestionNode.Create(1);
  Names::Add("CongestionSender", congestionNode.Get(0));

  DataRate b_access(bandwidth_access);
  DataRate b_bottleneck(bandwidth_bottleneck);
  Time d_access(delay_access);
  Time d_bottleneck(delay_bottleneck);
  Time d_serialization(delay_serialization);

  uint32_t max_bottleneck_bytes = static_cast<uint32_t>(
      ((std::min(b_access, b_bottleneck).GetBitRate() / 8) *
       (((d_access * 2) + d_bottleneck) * 2 + d_serialization).GetSeconds()));
  uint32_t projected_queue_max_packets =
      std::ceil(max_bottleneck_bytes / MTU_bytes);

  // Set Droptail queue size to 1 packet
  Config::SetDefault(SimulationQueue::getQueueString() + "::MaxSize", StringValue("100p"));

  // Create the point-to-point link helpers and connect two router nodes
  PointToPointHelper pointToPointRouter;
  pointToPointRouter.SetQueue(SimulationQueue::getQueueString());
  pointToPointRouter.SetDeviceAttribute("DataRate",
                                        StringValue(bandwidth_bottleneck));
  pointToPointRouter.SetChannelAttribute("Delay",
                                         StringValue(delay_bottleneck));



  // Draw a ring topology between R1 -> R2 -> R3
  /* 
   *               R3
   *              /  \
   *             /    \
   *     CS --- R1----R2 --- R
   */
  NetDeviceContainer r1r2ND =
      pointToPointRouter.Install(routers.Get(0), routers.Get(1));
  
  NetDeviceContainer r1r3ND =
      pointToPointRouter.Install(routers.Get(0), routers.Get(2));
  NetDeviceContainer r3r2ND =
      pointToPointRouter.Install(routers.Get(2), routers.Get(1));

  setAlternateTarget<0>(r1r2ND, getDevice<0>(r1r3ND));	
  //setAlternateTarget<0>(r1r2ND, getDevice<0>(devices02));	
  
  // Create the point-to-point link helpers and connect leaf nodes to router
  PointToPointHelper pointToPointLeaf;
  //pointToPointLeaf.SetQueue(SimulationQueue::getQueueString());
  pointToPointLeaf.SetDeviceAttribute("DataRate",
                                      StringValue(bandwidth_access));
  pointToPointLeaf.SetChannelAttribute("Delay", StringValue(delay_access));

  NetDeviceContainer leftToRouter =
      pointToPointLeaf.Install(leftNodes.Get(0), routers.Get(0));
  NetDeviceContainer routerToRight =
      pointToPointLeaf.Install(routers.Get(1), rightNodes.Get(0));

  // Link CongestioNSender to Router1
  NetDeviceContainer congestionSenderToRouter =
      pointToPointLeaf.Install(congestionNode.Get(0), routers.Get(0));

  InternetStackHelper internetStack;
  internetStack.Install(leftNodes);
  internetStack.Install(rightNodes);
  internetStack.Install(routers);
  internetStack.Install(congestionNode);

  Ipv4AddressHelper ipAddresses("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer r1r2IPAddress = ipAddresses.Assign(r1r2ND);
  ipAddresses.NewNetwork();
  Ipv4InterfaceContainer leftToRouterIPAddress =
      ipAddresses.Assign(leftToRouter);
  ipAddresses.NewNetwork();
  Ipv4InterfaceContainer routerToRightIPAddress =
      ipAddresses.Assign(routerToRight);
  ipAddresses.NewNetwork();
  Ipv4InterfaceContainer congestionToRouter0IPAddress =
      ipAddresses.Assign(congestionSenderToRouter);
  ipAddresses.NewNetwork();


  Ipv4InterfaceContainer r1r3IPAddress = ipAddresses.Assign(r1r3ND);
  ipAddresses.NewNetwork();
  Ipv4InterfaceContainer r3r2IPAddress = ipAddresses.Assign(r3r2ND);
  ipAddresses.NewNetwork();

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Config::SetDefault("ns3::PfifoFastQueueDisc::MaxSize",
  // QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS,
  // projected_queue_max_packets)));
  Config::SetDefault("ns3::PfifoFastQueueDisc::MaxSize",
                     QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, 100)));

  TrafficControlHelper tch;
  tch.SetRootQueueDisc("ns3::PfifoFastQueueDisc");
  QueueDiscContainer qd;
  tch.Uninstall(routers.Get(0)->GetDevice(0));
  qd.Add(tch.Install(routers.Get(0)->GetDevice(0)).Get(0));

  /* Trace the QueueDisc Queue size */
  if (storeTraces == false) {
    Ptr<QueueDisc> q = qd.Get(0);
    q->TraceConnectWithoutContext("PacketsInQueue",
                                  MakeCallback(&PacketsInQueueDisc));
  }

  /* Trace the DropTail Queue size */
  if (storeTraces) {
    Ptr<NetDevice> nd = routers.Get(0)->GetDevice(0);
    Ptr<PointToPointNetDevice> ptpnd = DynamicCast<PointToPointNetDevice>(nd);
    Ptr<Queue<Packet>> queue = ptpnd->GetQueue();
    queue->TraceConnectWithoutContext("PacketsInQueue",
                                      MakeCallback(&PacketsInDroptail));
  }

  /* Trace packets dropped by the QueueDisc Queue */
  if (storeTraces) {
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> streamWrapper;
    streamWrapper = ascii.CreateFileStream(tracesPath + "droppedQueueDisc.txt");
    qd.Get(0)->TraceConnectWithoutContext(
        "Drop", MakeBoundCallback(&DropAtQueue, streamWrapper));
  }

  netDeviceToDropPacket = &r1r2ND;
  /* Callback to the ExaminePacket */
  r1r2ND.Get(0)->TraceConnectWithoutContext("MacTx",
                                            MakeCallback(&ExaminePacket));

  /* Packet Printing is mandatory for the Packet Drop Test */
  Packet::EnablePrinting();

  uint16_t server_port = 50000;
  /* Install packet sink at receiver side */
  InstallPacketSink(rightNodes.Get(0), server_port, socketFactory,
                    Seconds(0.01), stopTimeSimulation);
  // InstallPacketSink(rightNodes.Get(0), server_port, "ns3::UdpSocketFactory",
  //                   Seconds(0.01), stopTimeSimulation);

  /* Install BulkSend application */
  // InstallBulkSend(leftNodes.Get(0), routerToRightIPAddress.GetAddress(1),
  //                 server_port, socketFactory, 2, 0,
  //                 MakeCallback(&CwndChange), bytes_to_send, Seconds(0.2));

  // Congestion PARAMETERS
  uint16_t serverPort = 50000;
  Time startDelay = Seconds(1.0);
  Time duration = Seconds(5.0);
  uint32_t numPackets = 512;
  uint32_t packetSize = 1024;
  Time interval = MilliSeconds(10);
  Ipv4Address receiverAddress = routerToRightIPAddress.GetAddress(1);

  // Schedule the congestion
  //   Simulator::Schedule(startDelay, [congestionNode, startDelay, duration,
  //                                    numPackets, packetSize, interval,
  //                                    receiverAddress, serverPort]() {
  //     Ptr<Node> node = congestionNode.Get(0);
  //     // double startDelaySeconds = startDelay.GetSeconds();
  //     // double duration
  //     SimulateCongestion(node, startDelay, duration, numPackets, packetSize,
  //                        interval, receiverAddress, serverPort);
  //   });

  // SimulateCongestion(congestionNode.Get(0), startDelay, duration, numPackets,
  //                 packetSize, interval, receiverAddress, serverPort);

  OnOffHelper onoff(
      "ns3::UdpSocketFactory",
      InetSocketAddress(routerToRightIPAddress.GetAddress(1), 50000));
  onoff.SetAttribute("PacketSize", UintegerValue(8));
  onoff.SetAttribute("DataRate", DataRateValue(DataRate("200kbps")));

  ApplicationContainer app = onoff.Install(congestionNode.Get(0));
  app.Start(Seconds(1.0));
  app.Stop(Seconds(3.0));

  if (storeTraces) {
    pointToPointRouter.EnablePcapAll(tracesPath);
  }

  // Simulator::Stop(stopTimeSimulation);
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
