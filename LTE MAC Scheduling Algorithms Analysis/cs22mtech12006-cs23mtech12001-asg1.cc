/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/mobility-helper.h"
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/radio-environment-map-helper.h"
#include "ns3/channel-list.h"
#include "ns3/random-walk-2d-mobility-model.h"
#include "ns3/config-store.h"
#include "ns3/gnuplot.h"
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("asg1 logging");  // Define the logging component

// Define a class to log the position of the nodes
class Logging{
    public:
    // Create a function to log the position
    static void LogPosition(Ptr<MobilityModel const> mobility) {
        Vector pos = mobility->GetPosition();   // Get the position of the node
        std::cout << "Node position: " << pos;
    }
};

// Define an enum to represent the scheduler type    
enum SchType{
    PF,   // Proportional Fair
    RR,   // Round Robin 
    MT,   // Max Throughput  
    BATS  // Blind Average Throughput Scheduler 
};

// define the function prototypes 
void setNodes(NodeContainer&, NodeContainer&, int, int);
void PrintNodePosition(ns3::Ptr<ns3::Node> node);
std::string schdulerToStr(bool);

// Define a function to log the attachment of the UE to the cell
void UeAttach (uint64_t imsi, uint16_t cellId)
{
  std::cout << "UE " << imsi << " attached to cell " << cellId << std::endl;
}

// Define the global variables to store the total transmitted and received bytes and the throughput values 
double speed = 5;   // Speed of the UE nodes in m/s                 
SchType schType;    // Scheduler type  
bool fullBufferFlag;   // Flag to indicate whether to use full buffer mode or not
std::ofstream stream; 

// ------------------------------------------------------- Main Function -------------------------------------------------------
int main(int argc, char* argv[])
{
    srand(time(0));
    uint64_t seed = rand() % 63453 + 1000;
    int schTypeInt;

   // -------------------------------------- Command Line Arguments --------------------------------------
    CommandLine cmd(__FILE__);
    cmd.AddValue("speed", "speed of the ueNodes", speed);
    cmd.AddValue("RngRun", "seed value for random position generation", seed);
    cmd.AddValue("schType", "scheduler type (RR=0, PF=1, MT=2, BATS=3)", schTypeInt);
    cmd.AddValue("fullBufferFlag", "use full buffer mode or not", fullBufferFlag);
    
    // Provide usage message
    cmd.Usage ("Provide the speed of the ueNodes, seed value for random position generation, scheduler type (PF=0, RR=1, MT=2, BATS=3), and whether to use full buffer mode or not, Example: ./ns3 run asg1 --speed=5 --RngRun=1 --schType=0 --fullBufferFlag=1");

    // Parse the command line arguments 
    cmd.Parse(argc, argv);
    std::cout << "rngRun seed " << seed << " speed " << speed  << " schType " << schTypeInt << " fullbuffer " << fullBufferFlag << std::endl;
    
    // Convert the integer value of the scheduler type to the enum type
    switch (schTypeInt)
    {
        case 0:
            schType = PF;
            break;
        case 1:
            schType = RR;
            break;
        case 2:
            schType = MT;
            break;
        case 3:
            schType = BATS;
            break;
        default:
            break;
    }

    //--------------------------------------------------- Setup Network Topology ---------------------------------------------------

    /*  Network Topology as given in the assignment with 4 eNBs at 1km distance from each other. 
                C--------D
                |        |
                |        |
    RH----PG----A--------B
    */

    double dstInMeters = 1000; //1km
    NodeContainer enbNodes;
    enbNodes.Create(4);
    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults();

    //Placing eNBs at the corners of the square with 1km distance between them
    Ptr<ListPositionAllocator> enbpositionAlloc = CreateObject<ListPositionAllocator>();
    enbpositionAlloc->Add(Vector(0, 0, 0)); // Position A (0,0,0)
    enbpositionAlloc->Add(Vector(dstInMeters, 0, 0)); // Position B (1000,0,0)
    enbpositionAlloc->Add(Vector(0, dstInMeters, 0)); // Position C (0,1000,0)
    enbpositionAlloc->Add(Vector(dstInMeters, dstInMeters, 0)); // Position D (1000,1000,0)
    
    //Setting mobility of eNBs
    MobilityHelper mobilityHelper;
    mobilityHelper.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityHelper.SetPositionAllocator(enbpositionAlloc);
    mobilityHelper.Install(enbNodes);

    // Create the UE nodes (20 UEs in total, 5 UEs per eNB)
    NodeContainer ueNodes;
    ueNodes.Create(20);

    //Placing UEs at random positions within the range of 500m of the eNBs 
    NodeContainer ueNodesA;     
    NodeContainer ueNodesB;
    NodeContainer ueNodesC;
    NodeContainer ueNodesD;

    //Setting 5 UEs per eNB (initial position setup)
    setNodes(ueNodesA, ueNodes, 0, 5);  
    setNodes(ueNodesB, ueNodes, 5, 10); 
    setNodes(ueNodesC, ueNodes, 10, 15);
    setNodes(ueNodesD, ueNodes, 15, 20);

    //Setting mobility of UEs
    std::stringstream speedStr;
    double speedL = (speed == 0) ? 0.001 : speed; 
    speedStr << "ns3::ConstantRandomVariable[Constant=" << speedL <<  "]";
    mobilityHelper.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                    "Speed", StringValue(speedStr.str()),
                                    "Bounds",StringValue("-4000|4000|-4000|4000"));  
    
    std::string coord = std::to_string(dstInMeters);
    ObjectFactory positionAllocatorFactory;
    Ptr<PositionAllocator> positionAllocator;
   
    // Set the seed value for the random number generator
    RngSeedManager::SetSeed(seed);

    // Assign random positions to the UEs of eNB at position A (0,0,0)
    positionAllocatorFactory.SetTypeId("ns3::RandomDiscPositionAllocator"); 
    positionAllocatorFactory.Set("X", StringValue("0.0"));
    positionAllocatorFactory.Set("Y", StringValue("0.0"));
    positionAllocatorFactory.Set("Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=500]"));  // Random position within 500m of the eNB
    
    positionAllocator = positionAllocatorFactory.Create()->GetObject<PositionAllocator>();
    mobilityHelper.SetPositionAllocator(positionAllocator);
    
    mobilityHelper.Install(ueNodesA);
    
    // Assign random positions to the UEs of eNB at position B (1000,0,0)
    positionAllocatorFactory.SetTypeId("ns3::RandomDiscPositionAllocator");
    positionAllocatorFactory.Set("X", StringValue(coord));
    positionAllocatorFactory.Set("Y", StringValue("0.0"));
    positionAllocatorFactory.Set("Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=500]"));
    
    positionAllocator = positionAllocatorFactory.Create()->GetObject<PositionAllocator>();
    mobilityHelper.SetPositionAllocator(positionAllocator);
    mobilityHelper.Install(ueNodesB);
    
    // Assign random positions to the UEs of eNB at position C (0,1000,0)
    positionAllocatorFactory.SetTypeId("ns3::RandomDiscPositionAllocator");
    positionAllocatorFactory.Set("X", StringValue("0.0"));
    positionAllocatorFactory.Set("Y", StringValue(coord));
    positionAllocatorFactory.Set("Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=500]"));
    
    positionAllocator = positionAllocatorFactory.Create()->GetObject<PositionAllocator>();
    mobilityHelper.SetPositionAllocator(positionAllocator);
    mobilityHelper.Install(ueNodesC);
                                                              
    // Assign random positions to the UEs of eNB at position D (1000,1000,0)
    positionAllocatorFactory.SetTypeId("ns3::RandomDiscPositionAllocator");
    positionAllocatorFactory.Set("X", StringValue(coord));
    positionAllocatorFactory.Set("Y", StringValue(coord));
    positionAllocatorFactory.Set("Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=500]"));
    
    positionAllocator = positionAllocatorFactory.Create()->GetObject<PositionAllocator>();
    mobilityHelper.SetPositionAllocator(positionAllocator);
    mobilityHelper.Install(ueNodesD);

    // ------------------------------------------------------ Setup LTE Network ------------------------------------------------------

    // <------ A. Create the LTE Helper and the EPC Helper and algorithm setup ------->
    // Create the LTE helper and the EPC helper
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper); 

    // Enable logging of the components
    lteHelper->EnableLogComponents(); 

    // Configuring the handover algorithm with the A3 RSRP algorithm 
    lteHelper->SetHandoverAlgorithmType("ns3::A3RsrpHandoverAlgorithm");
    
    // Configur scheduler algorithm
    lteHelper->SetSchedulerType(schdulerToStr(true));

    // Set the pathloss model to the Friis Spectrum Propagation Loss Model
    lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));
  
    // initialize the nodes for the LTE network
    for (auto it = ueNodes.Begin(); it != ueNodes.End(); ++it)
    {
        (*it)->Initialize();
    }

    mobilityHelper.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    //<------ B. Create PGW and Remote Host and set up the internet connection ------->
    // Create the PGW node and set its position
    Ptr<Node> pgw = epcHelper->GetPgwNode();
    Ptr<ListPositionAllocator> pgwpositionAlloc = CreateObject<ListPositionAllocator>();
    pgwpositionAlloc->Add(Vector(-250,0,0));    
    mobilityHelper.SetPositionAllocator(pgwpositionAlloc);
    mobilityHelper.Install(pgw);

    // Create the remote host node and set its position
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    InternetStackHelper internet;
    internet.Install(remoteHostContainer);
    Ptr<ListPositionAllocator> rHpositionAlloc = CreateObject<ListPositionAllocator>();
    rHpositionAlloc->Add(Vector(-500,0,0));
    mobilityHelper.SetPositionAllocator(rHpositionAlloc);
    mobilityHelper.Install(remoteHostContainer);

    // Create the internet connection between the PGW and the remote host
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("1Gb/s")));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
    p2ph.SetChannelAttribute("Delay", TimeValue(Seconds (0.010)));
    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);

    // Assign IP addresses to the PGW and the remote host
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);

    // Set the default gateway for the remote host
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);  

    // <------ C. Install LTE Devices to the nodes and set up the IP stack ------->
    // Install the LTE devices to the eNB nodes and the UE nodes
    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
    NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);
    Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(30));

    // Set the Tx power, DL and UL bandwidths for the eNBs 
    for(uint32_t u = 0; u < enbLteDevs.GetN(); u++)
    {
        enbLteDevs.Get(u)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(30.0); //Tx Power
        enbLteDevs.Get(u)->GetObject<LteEnbNetDevice>()->SetDlBandwidth(50); // DL RBs 
        enbLteDevs.Get(u)->GetObject<LteEnbNetDevice>()->SetUlBandwidth(50); //UL RBs
    }

    // Install the IP stack to the nodes and assign IP addresses to the UEs and the remote host. 
    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));
    lteHelper->AddX2Interface(enbNodes);

    std::cout << "\n UE IpAddress: " << ueIpIface.GetAddress(0) << "\n"; //testing purpose
    std::cout << "\n RH IpAddress: " << internetIpIfaces.GetAddress(0) << "\n"; //testing purpose
    
    // <------ D. Set up the routing and automatically attach the UEs to the eNBs ------->
    // Set the default gateway for the UEs
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u)
    {
        Ptr<Node> ueNode = ueNodes.Get(u);
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
    }

    std::cout << "\n Default Router Address of UE's: " << epcHelper->GetUeDefaultGatewayAddress() << "\n"; //testing purpose

    // Automatically attach the UEs to the eNBs based on the signal strength (RSRP)
    
    lteHelper->Attach(ueLteDevs); 

    // <------ E. Set up the applications and start the simulation ------->
    // Install and start applications on UEs and remote host
    uint16_t dlPort = 1234; // Downlink port number

    ApplicationContainer clientApps;    // Container to store the client applications
    ApplicationContainer serverApps;    // Container to store the server applications

    // Install a PacketSink application on the remote host
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u)
    {
        PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), dlPort));
        serverApps.Add(sink.Install(ueNodes.Get(u)));

        // Install a UdpClient application on the UEs
        UdpClientHelper dlclient(ueIpIface.GetAddress(u), dlPort);
        
        // Set the attributes of the UdpClient application based on the full buffer flag
        if(fullBufferFlag)
        {
            dlclient.SetAttribute("MaxPackets", UintegerValue(1000000));  
            dlclient.SetAttribute("Interval", TimeValue(MilliSeconds(1)));
            dlclient.SetAttribute("PacketSize", UintegerValue(1500)); 
            clientApps.Add(dlclient.Install(remoteHost));
        }
        else
        {
            dlclient.SetAttribute("MaxPackets", UintegerValue(1000000));
            dlclient.SetAttribute("Interval", TimeValue(MilliSeconds(10)));
            dlclient.SetAttribute("PacketSize", UintegerValue(1500));  
            clientApps.Add(dlclient.Install(remoteHost));
        }
    }

    std::cout << "\n No. of Server Apps: " << serverApps.GetN() << "\n"; //testing purpose
    serverApps.Get(0)->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    std::cout << "\n Ip Address of Server App: " << serverApps.Get(5)->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << std::endl; //testing purpose

    //starting app and stoping applications
    serverApps.Start(Seconds (0.01));
    clientApps.Start(Seconds (0.01));
    p2ph.EnablePcapAll("asg1");   

    // <------ F. (Optional) Generate the Radio Environment Map (REM) and start the simulation ------->
    Ptr<RadioEnvironmentMapHelper> remHelper;
    BooleanValue generateRem = false;     // Flag to indicate whether to generate the REM or not 
    if (generateRem)
    {
        remHelper = CreateObject<RadioEnvironmentMapHelper>();
        remHelper->SetAttribute("Channel", PointerValue(lteHelper->GetDownlinkSpectrumChannel()));
        remHelper->SetAttribute("OutputFile", StringValue("asg1-rem.out"));
        remHelper->SetAttribute("XMin", DoubleValue(-4000.0));
        remHelper->SetAttribute("XMax", DoubleValue(4000.0));
        remHelper->SetAttribute("XRes", UintegerValue(500));
        remHelper->SetAttribute("YMin", DoubleValue(-4000.0));
        remHelper->SetAttribute("YMax", DoubleValue(4000.0));
        remHelper->SetAttribute("YRes", UintegerValue(500));
        remHelper->SetAttribute("Z", DoubleValue(0.0));
        remHelper->SetAttribute("UseDataChannel", BooleanValue(true));
        remHelper->SetAttribute("RbId", IntegerValue(-1));

        remHelper->Install();
    }

    // <------ G. Set up the flow monitor and start the simulation -------> 
    FlowMonitorHelper flowmon;  
    Ptr<FlowMonitor> Monitor;
    Monitor = flowmon.Install(ueNodes);
    Monitor = flowmon.Install(remoteHostContainer);
    
    // Schedule the function to print the position of the nodes
    ns3::Time printTime = ns3::Seconds(1.0); 
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u)
    {
        ns3::Simulator::Schedule(printTime, &PrintNodePosition, ueNodes.Get(u));
    }
    Simulator::Stop(Seconds(10));

    lteHelper->EnableRlcTraces();
    Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats();
    // file name should be <sceduler name>-<speed>-<bufer mode>-<RngRun>.csv store at directory Data
    rlcStats->SetDlOutputFilename("Data/DlRlcStats-" + schdulerToStr(false) + "-" + std::to_string((int)speed) + "-" + std::to_string(fullBufferFlag) + "-" + std::to_string(seed) + ".csv");
    NS_LOG_DEBUG(rlcStats);
    NS_LOG_DEBUG("File name: " << "Data/DlRlcStats-" + schdulerToStr(false) + "-" + std::to_string((int)speed) + "-" + std::to_string(fullBufferFlag) + "-" + std::to_string(seed) + ".csv");
    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

// ------------------------------------------------------- Function Definitions -------------------------------------------------------

// Function to set the nodes in the container 
void setNodes(NodeContainer &container, NodeContainer &ueNodes, int from, int to)
{
    for (int i = from; i < to; ++i) {
        container.Add(ueNodes.Get(i));
    }
}

// Function to print the position of the nodes 
void PrintNodePosition(ns3::Ptr<ns3::Node> node) {
  ns3::Ptr<ns3::MobilityModel> mobility = node->GetObject<ns3::MobilityModel>();
  ns3::Vector3D position = mobility->GetPosition();
  std::cout << "Node position: X=" << position.x << " Y=" << position.y << " Z=" << position.z << std::endl;
}

// Function to convert the scheduler type to a string
std::string schdulerToStr(bool isns3)
{
    if(!isns3)
    {
        switch (schType)
        {
            case PF:
                return "PF";  
            case RR:
                return "RR";  
            case MT:
                return "MT";
            case BATS:
                return "BATS";
        }
    }
    else
    {
        switch (schType)
        {
             case PF:
                return "ns3::PfFfMacScheduler";
            case RR:
                return "ns3::RrFfMacScheduler";    
            case MT:
               return "ns3::FdMtFfMacScheduler";
            case BATS:
                return "ns3::FdBetFfMacScheduler";
        }
    }
}
