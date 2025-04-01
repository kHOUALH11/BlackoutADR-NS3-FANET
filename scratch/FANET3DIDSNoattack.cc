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
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FanetSimulation");

// Global variables for CSV logging
std::ofstream csvFile;
uint32_t packetCount = 0;
std::map<uint32_t, double> batteryLevels; // Battery level map for each node

// Function to initialize battery levels
void InitializeBatteryLevels(NodeContainer nodes) {
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        batteryLevels[i] = 100.0; // Each node starts with 100% battery
    }
}

// Function to simulate battery depletion
void DepleteBattery(uint32_t nodeId, double txPower) {
    double drainRate = (txPower > 10.0) ? 0.5 : 0.2; // Higher drain rate for high power
    batteryLevels[nodeId] -= drainRate;
    if (batteryLevels[nodeId] < 0) {
        batteryLevels[nodeId] = 0; // Ensure battery level does not go below 0
    }
}

// Function to periodically log the current transmission power, battery level, and other data
void LogData(Ptr<Node> node, Ptr<WifiPhy> phy, double rssi) {
    double currentTime = Simulator::Now().GetSeconds();
    double txPowerStart = phy->GetTxPowerStart();
    uint32_t nodeId = node->GetId();

    // Deplete battery based on current transmission power
    DepleteBattery(nodeId, txPowerStart);

    // Log to CSV: Time, Node ID, RSSI, Transmission Power, Packet Count, Battery Level
    csvFile << currentTime << "," << nodeId << "," << rssi << ","
            << txPowerStart << "," << packetCount << "," << batteryLevels[nodeId] << "\n";
}

// Function to handle periodic logging, ADR adjustments, and battery depletion
void PeriodicLogAndAdr(Ptr<Node> node, Ptr<WifiNetDevice> dev) {
    Ptr<WifiRemoteStationManager> stationManager = dev->GetRemoteStationManager();
    Ptr<WifiPhy> phy = dev->GetPhy();

    // Simulate RSSI and adjust ADR based on it
    double simulatedRssi = -90 + (std::rand() % 20);
    if (simulatedRssi < -85) {
        stationManager->SetAttribute("DataMode", StringValue("DsssRate1Mbps"));
        phy->SetTxPowerStart(20.0);
        phy->SetTxPowerEnd(20.0);
        NS_LOG_UNCOND("Node " << node->GetId() << " setting to low data rate with high power");
    } else {
        stationManager->SetAttribute("DataMode", StringValue("DsssRate11Mbps"));
        phy->SetTxPowerStart(10.0);
        phy->SetTxPowerEnd(10.0);
        NS_LOG_UNCOND("Node " << node->GetId() << " setting to high data rate with low power");
    }

    // Log data to CSV
    LogData(node, phy, simulatedRssi);
    
    // Schedule next logging and ADR adjustment
    Simulator::Schedule(Seconds(1.0), &PeriodicLogAndAdr, node, dev);
}

// IDS Packet Sniffer for packet count
void PacketSniffer(Ptr<const Packet> packet) {
    packetCount++;
    NS_LOG_UNCOND("Leader Node 0 sniffed a packet of size: " << packet->GetSize() << " bytes");
}

// Install IDS on the leader node
void InstallIds(NodeContainer nodes) {
    Ptr<Node> leaderNode = nodes.Get(0); // Leader Node 0
    leaderNode->GetDevice(0)->TraceConnectWithoutContext("PhyRxEnd", MakeCallback(&PacketSniffer));
}

int main (int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    NodeContainer nodes;
    nodes.Create(20);

    // Set up WiFi
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate54Mbps"));

    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel(wifiChannel.Create());

    NetDeviceContainer devices = wifi.Install(wifiPhy, mac, nodes);

    // Install Internet and Routing
    AodvHelper aodv;
    InternetStackHelper internet;
    internet.SetRoutingHelper(aodv);
    internet.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    // Setup Mobility
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
                                   "X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
                                   "Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
                                   "Z", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
    mobility.SetMobilityModel("ns3::GaussMarkovMobilityModel",
                               "Bounds", BoxValue(Box(0, 100, 0, 100, 0, 100)),
                               "TimeStep", TimeValue(Seconds(0.5)),
                               "Alpha", DoubleValue(0.85),
                               "MeanVelocity", StringValue("ns3::UniformRandomVariable[Min=10|Max=20]"),
                               "MeanDirection", StringValue("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
                               "MeanPitch", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=0.05]"),
                               "NormalVelocity", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.1]"),
                               "NormalDirection", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2]"),
                               "NormalPitch", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02]"));
    mobility.Install(nodes);

    // Setup Applications
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(120.0));

    for (uint32_t i = 1; i < nodes.GetN(); ++i) {
        UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));
        ApplicationContainer clientApps = echoClient.Install(nodes.Get(i));
        clientApps.Start(Seconds(2.0 + i * 0.1));
        clientApps.Stop(Seconds(120.0));
    }

    // Open CSV file for logging
    csvFile.open("fanet_data__no_attack_New.csv");
    csvFile << "Time(s),Node,RSSI(dBm),TransmissionPower(dBm),PacketCount,BatteryLevel(%)\n";

    // Initialize battery levels
    InitializeBatteryLevels(nodes);

    // Schedule periodic logging and ADR adjustments
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        Ptr<Node> node = nodes.Get(i);
        Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice>(node->GetDevice(0));
        if (wifiDevice) {
            Simulator::Schedule(Seconds(1.0), &PeriodicLogAndAdr, node, wifiDevice);
        }
    }

    // Install IDS on the leader node
    InstallIds(nodes);

    // Enable Pcap, XML, and ASCII tracing
    wifiPhy.EnablePcap("Fanetleader1", devices, true); // Enable pcap on all nodes

    AnimationInterface anim("Fanet3DLoraWanleadernewsimulation.xml");
    anim.SetConstantPosition(nodes.Get(0), 50, 50); // Positioning Node 0 (Leader Node)
    anim.UpdateNodeColor(nodes.Get(0), 0, 255, 0); // Set Leader Node color to green

    AsciiTraceHelper ascii;
    wifiPhy.EnableAsciiAll(ascii.CreateFileStream("Fanet3D.tr"));

    Simulator::Stop(Seconds(120.0));
    Simulator::Run();
    csvFile.close();
    Simulator::Destroy();

    return 0;
}
