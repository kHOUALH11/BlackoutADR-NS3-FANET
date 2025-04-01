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
#include <cstdlib> // For system() calls to run ML and Snort

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FanetMLSnortSimulation");

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
void DepleteBattery(uint32_t nodeId, double txPower, bool underAttack) {
    double drainRate = (txPower > 10.0) ? 0.5 : 0.2; // Normal drain rate
    if (underAttack) {
        drainRate *= 2; // Double drain rate during ADR attack
    }
    batteryLevels[nodeId] -= drainRate;
    if (batteryLevels[nodeId] < 0) {
        batteryLevels[nodeId] = 0; // Ensure battery level does not go below 0
    }
}

// Function to periodically log metrics to a CSV
void LogMetrics(Ptr<Node> node, Ptr<WifiPhy> phy, bool underAttack) {
    double currentTime = Simulator::Now().GetSeconds();
    double txPowerStart = phy->GetTxPowerStart();
    uint32_t nodeId = node->GetId();
    double adjustedTxPower = underAttack ? txPowerStart * 1.5 : txPowerStart; // Simulate ADR

    DepleteBattery(nodeId, adjustedTxPower, underAttack);

    csvFile << currentTime << "," << nodeId << "," << adjustedTxPower << "," << batteryLevels[nodeId] << ","
            << (underAttack ? "UnderAttack" : "Normal") << "\n";
}

// Function to analyze PCAP with the ML model
// Function to analyze PCAP with the ML model
void AnalyzePcapWithMl(const std::string& pcapFile) {
    // Use bash explicitly to source the virtual environment and run the Python script
    std::string command = "bash -c 'source /home/hidawi/tf_env/bin/activate && python3 process_pcap_with_ml.py --pcap " + pcapFile + "'";
    int result = system(command.c_str());
    if (result == 0) {
        NS_LOG_UNCOND("ML analysis completed successfully.");
    } else {
        NS_LOG_UNCOND("ML analysis failed. Check logs for errors.");
    }
}




// Function to run Snort on the PCAP file
void RunSnortAnalysis() {
    std::string command = "snort -r FanetleaderML-0-0.pcap -c /etc/snort/snort.lua -A console";
    int result = system(command.c_str());
    if (result == 0) {
        NS_LOG_UNCOND("Snort analysis completed successfully.");
    } else {
        NS_LOG_UNCOND("Snort analysis failed. Check logs.");
    }
}

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Create nodes
    NodeContainer nodes;
    nodes.Create(10);

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

    // Internet and Routing Setup
    AodvHelper aodv;
    InternetStackHelper internet;
    internet.SetRoutingHelper(aodv);
    internet.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    // Mobility Setup
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue(0.0),
                                   "MinY", DoubleValue(0.0),
                                   "DeltaX", DoubleValue(20.0),
                                   "DeltaY", DoubleValue(20.0),
                                   "GridWidth", UintegerValue(5),
                                   "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Applications
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(100.0));

    for (uint32_t i = 1; i < nodes.GetN(); ++i) {
        UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(100));
        ApplicationContainer clientApps = echoClient.Install(nodes.Get(i));
        clientApps.Start(Seconds(2.0 + i * 0.1));
        clientApps.Stop(Seconds(100.0));
    }

    // Open CSV for logging
    csvFile.open("FanetSimulationMetrics.csv");
    csvFile << "Time(s),NodeId,TxPower(dBm),BatteryLevel(%),Status\n";

    // Initialize battery levels
    InitializeBatteryLevels(nodes);

    // Schedule periodic logging
    bool underAttack = true; // Enable ADR attack simulation
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        Ptr<Node> node = nodes.Get(i);
        Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice>(node->GetDevice(0));
        if (wifiDevice) {
            Simulator::Schedule(Seconds(1.0), &LogMetrics, node, wifiDevice->GetPhy(), underAttack);
        }
    }

    // Enable Pcap tracing for leader node
    wifiPhy.EnablePcap("FanetleaderML", devices.Get(0), true);

    // Simulation
    Simulator::Stop(Seconds(100.0));
    Simulator::Run();

    // Run Snort and ML analysis
    AnalyzePcapWithMl("FanetleaderML-0-0.pcap");
   RunSnortAnalysis();

    Simulator::Destroy();

    // Close CSV file
    csvFile.close();

    return 0;
}
