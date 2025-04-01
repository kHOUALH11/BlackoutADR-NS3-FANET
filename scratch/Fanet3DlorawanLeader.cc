#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/aodv-helper.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

// Define the logging component
NS_LOG_COMPONENT_DEFINE ("FanetSimulation");

// ADR adjustment function for LoraWan
void AdjustAdr (Ptr<Node> node, Ptr<WifiNetDevice> dev)
{
    Ptr<WifiRemoteStationManager> stationManager = dev->GetRemoteStationManager();
    Ptr<WifiPhy> phy = dev->GetPhy();

    double simulatedRssi = -90 + (std::rand() % 20);  // Simulate RSSI between -90 and -70 dBm
    NS_LOG_UNCOND ("Node " << node->GetId() << " RSSI: " << simulatedRssi << " dBm");

    if (simulatedRssi < -85)
    {
        // Low RSSI: Reduce data rate, increase transmission power
        stationManager->SetAttribute("DataMode", StringValue("DsssRate1Mbps"));
        phy->SetTxPowerStart(20.0);
        phy->SetTxPowerEnd(20.0);
        NS_LOG_UNCOND ("Node " << node->GetId() << " setting to low data rate with high power");
    }
    else
    {
        // High RSSI: Increase data rate, reduce transmission power
        stationManager->SetAttribute("DataMode", StringValue("DsssRate11Mbps"));
        phy->SetTxPowerStart(10.0);
        phy->SetTxPowerEnd(10.0);
        NS_LOG_UNCOND ("Node " << node->GetId() << " setting to high data rate with low power");
    }
}

// Packet sniffer for leader node (Node 0)
void PacketSniffer(Ptr<const Packet> packet)
{
    NS_LOG_UNCOND("Leader Node 0 sniffed a packet of size: " << packet->GetSize() << " bytes");
}

void InstallIds(NodeContainer nodes)
{
    Ptr<Node> leaderNode = nodes.Get(0); // Designate Node 0 as the leader
    leaderNode->GetDevice(0)->TraceConnectWithoutContext("PhyRxEnd", MakeCallback(&PacketSniffer));
}

int main (int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse (argc, argv);

    NodeContainer nodes;
    nodes.Create (20);

    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211b);

    WifiMacHelper mac;
    mac.SetType ("ns3::AdhocWifiMac");

    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate54Mbps"));

    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel (wifiChannel.Create ());

    NetDeviceContainer devices = wifi.Install (wifiPhy, mac, nodes);

    AodvHelper aodv;
    InternetStackHelper internet;
    internet.SetRoutingHelper (aodv);
    internet.Install (nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
                                   "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
                                   "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
                                   "Z", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));

    mobility.SetMobilityModel ("ns3::GaussMarkovMobilityModel",
                               "Bounds", BoxValue (Box (0, 100, 0, 100, 0, 100)),
                               "TimeStep", TimeValue (Seconds (0.5)),
                               "Alpha", DoubleValue (0.85),
                               "MeanVelocity", StringValue ("ns3::UniformRandomVariable[Min=10|Max=20]"),
                               "MeanDirection", StringValue ("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
                               "MeanPitch", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=0.05]"),
                               "NormalVelocity", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.1]"),
                               "NormalDirection", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2]"),
                               "NormalPitch", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02]"));
    mobility.Install (nodes);

    // Setup applications: All nodes communicate with the leader node (Node 0)
    UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApps = echoServer.Install (nodes.Get (0)); // Leader node as server
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));

    for (uint32_t i = 1; i < nodes.GetN(); ++i)
    {
        UdpEchoClientHelper echoClient (interfaces.GetAddress (0), 9); // All clients communicate with Node 0
        echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
        echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
        echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
        ApplicationContainer clientApps = echoClient.Install (nodes.Get (i));
        clientApps.Start (Seconds (2.0 + i * 0.1));
        clientApps.Stop (Seconds (10.0));
    }

    wifiPhy.EnablePcapAll ("Fanet3D");
    AnimationInterface anim ("Fanet3DLoraWanleader.xml");

    AsciiTraceHelper ascii;
    wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("Fanet3D.tr"));

    // Schedule ADR adjustments for each node
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<Node> node = nodes.Get(i);
        Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice>(node->GetDevice(0));
        if (wifiDevice)
        {
            Simulator::Schedule(Seconds(1.0), &AdjustAdr, node, wifiDevice);
        }
    }

    // Install IDS on the leader node to capture all packets
    InstallIds(nodes);

    Simulator::Stop (Seconds (10.0));
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
