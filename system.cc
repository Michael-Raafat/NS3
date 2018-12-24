#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"

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
/*
#define CONNECTION_NUMBER 6
static const int connections[][2] = {
  {0, 1}, {0, 4}, {1, 2}, {2, 3}, {3, 5}, {4, 5}
};
*/
NS_LOG_COMPONENT_DEFINE ("Our topology");

NetDeviceContainer makeP2Pconnection(int i, int j, Ptr<Node>[]);

int main (int argc, char *argv[]) {
  CommandLine cmd;
  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
  //LogComponentEnable("GlobalRouteManagerImpl", LOG_LEVEL_LOGIC);
  // Construct basic toplogy.
  NodeContainer nodes;
  Ptr<Node> nodesP[DEVICE_NUMBER];
  vector<Ipv4Address> nodesIP[DEVICE_NUMBER];
  vector<uint32_t> nodesInter[DEVICE_NUMBER];
  nodes.Create (DEVICE_NUMBER);
  for (uint32_t i = 0; i < nodes.GetN(); i++) {
  	nodesP[i] = nodes.Get(i);
    nodesIP[i] = vector<Ipv4Address>();
    nodesInter[i] = vector<uint32_t>();
  }

  vector<NetDeviceContainer> devices;

  cout << "---------------------------------------------------------------------------------" << endl;
  for (uint32_t i = 0; i < CONNECTION_NUMBER; i++) {
    NetDeviceContainer device = makeP2Pconnection(connections[i][0], connections[i][1], nodesP);
    devices.push_back(device);
    nodesP[connections[i][0]]->AddDevice(device.Get(0));
    nodesP[connections[i][1]]->AddDevice(device.Get(1));
  }
  cout << "---------------------------------------------------------------------------------" << endl;


  // Construct Network Layer.
  InternetStackHelper stack;
  stack.Install (nodes);
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer interfaces;
  for (uint32_t in = 0; in < devices.size(); in++) {
    string net_domain = string("10.1." + to_string(in + 1) + ".0");
    address.SetBase (net_domain.c_str(), "255.255.255.0");
    int i = devices[in].Get(0)->GetNode()->GetId();
    int j = devices[in].Get(1)->GetNode()->GetId();
    cout << net_domain << " assigned to Point-to-point connection between " << i << " and " << j << endl;
    Ipv4InterfaceContainer interface = address.Assign(devices[in]);
    interfaces.Add(interface);
    nodesIP[i].push_back(interface.GetAddress(0));
    nodesIP[j].push_back(interface.GetAddress(1));
    nodesInter[i].push_back(interface.Get(0).second);
    nodesInter[j].push_back(interface.Get(1).second);
  }
  cout << "---------------------------------------------------------------------------------" << endl;
  cout << "Devices and IP's" << endl;
  for (uint32_t i = 0; i < nodes.GetN(); i++) {
    cout << "\tNode " << i << " : " << endl;
    for (uint32_t j = 0; (j) < nodesIP[i].size(); j++) {
      cout << "\t\tInterface " << nodesInter[i][j] << " : " << nodesIP[i][j] << endl;
    }
  }
  cout << "---------------------------------------------------------------------------------" << endl;
  for (uint32_t i = 0; i < nodes.GetN(); i++) {
  	Ptr<GlobalRouter> rtr =
        nodesP[i]->GetObject<GlobalRouter> ();
  	cout << "Node " << i << " has router ip of " << rtr->GetRouterId () << endl;
  }
  cout << "---------------------------------------------------------------------------------" << endl;

  // Establish Routes and print them.
  cout << "SPF trees : " << endl;
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  OutputStreamWrapper wrapper = OutputStreamWrapper(&std::cout);
  Ipv4GlobalRoutingHelper::PrintRoutingTableAllAt(Time(), &wrapper, Time::NS);


  // Application layer to test topology.
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (5));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces.GetAddress (interfaces.GetN() - 1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));


  // Animation to see in gui our system.
  AnimationInterface anim ("system.xml");
  anim.SetConstantPosition (nodes.Get(0), 0.0, 10.0);
  anim.SetConstantPosition (nodes.Get(1), 20.0, 20.0);
  anim.SetConstantPosition (nodes.Get(2), 20.0, 0.0);
  anim.SetConstantPosition (nodes.Get(3), 30.0, 20.0);
  anim.SetConstantPosition (nodes.Get(4), 30.0, 0.0);
  anim.SetConstantPosition (nodes.Get(5), 40.0, 0.0);
  cout << "---------------------------------------------------------------------------------" << endl;
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
