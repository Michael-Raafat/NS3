#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipv4-global-routing-helper.h"

#include <iostream>
#include <vector>

#define	DATA_RATE	"1Mbps"
#define PROPAGATION_DELAY	"1ms"
#define DEVICE_NUMBER	6

using namespace ns3;
using namespace std;

#define CONNECTION_NUMBER 8
static const int connections[][2] = {
  {0, 1}, {0, 2}, {1, 2}, {1, 3}, {1, 4}, {2, 4}, {3, 4}, {4, 5}
};


NS_LOG_COMPONENT_DEFINE ("Our topology");

NetDeviceContainer makeP2Pconnection(int i, int j, Ptr<Node>[]);

int main (int argc, char *argv[]) {
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
  NodeContainer nodes;
  Ptr<Node> nodesP[DEVICE_NUMBER];
  nodes.Create (DEVICE_NUMBER);
  for (uint32_t i = 0; i < nodes.GetN(); i++) {
  	nodesP[i] = nodes.Get(i);
  }
  
  vector<NetDeviceContainer> devices;  
  for (uint32_t i = 0; i < CONNECTION_NUMBER; i++) {
    NetDeviceContainer device = makeP2Pconnection(connections[i][0], connections[i][1], nodesP);
    devices.push_back(device);
    nodesP[connections[i][0]]->AddDevice(device.Get(0));
    nodesP[connections[i][1]]->AddDevice(device.Get(1));
  }
  
  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  Ipv4InterfaceContainer interfaces;
  for (uint32_t i = 0; i < devices.size(); i++) {
    address.SetBase (string("10.1." + to_string(i + 1) + ".0").c_str(), "255.255.255.0");
    Ipv4InterfaceContainer interface = address.Assign(devices[i]);
    interfaces.Add(interface);
  }
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  OutputStreamWrapper wrapper = OutputStreamWrapper(&std::cout);
  interfaces.Get(0).first->GetRoutingProtocol()->PrintRoutingTable(&wrapper, Time::NS);
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

NetDeviceContainer makeP2Pconnection(int i, int j, Ptr<Node> ptrs[]) {
	NodeContainer e = NodeContainer();
	e.Add(ptrs[i]);
	e.Add(ptrs[j]);
  cout << "Point-to-point established between nodes " << i << " and " << j << endl;
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue (DATA_RATE));
	pointToPoint.SetChannelAttribute ("Delay", StringValue (PROPAGATION_DELAY));
	return pointToPoint.Install(e);
}