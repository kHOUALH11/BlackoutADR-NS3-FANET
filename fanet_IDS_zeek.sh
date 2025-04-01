#!/bin/bash
# Step 1: Activate virtual environment
source /home/hidawi/tf_env/bin/activate
# Run ns-3 FANET simulation with attack
echo "Starting FANET simulation with attack..."
./ns3 run scratch/FANET3DIDSZeekML.cc &

# Allow the FANET simulation to run and generate the PCAP file
echo "Waiting for FANET simulation to generate the PCAP file..."
#sleep 10

# Convert the PCAP file to Ethernet format (if needed by Zeek)
#echo "Converting the PCAP file to Ethernet format for Zeek..."
#editcap -T ether Fanetleaderzeek-0-0.pcap Fanetleaderzeek-0-0-ethernet.pcap


#echo "Simulation and attack completed."

#./ns3 run scratch/IDSleaderFANET3D.cc

# Convert to Ethernet format
#editcap -T ether Fanetleader1-0-0.pcap Fanetleader1-0-0-ethernet.pcap

editcap -T ether FanetleaderML-0-0.pcap FanetleaderML-0-0-ethernet.pcap

# Run Snort on the converted pcap file
#sudo snort -c /etc/snort/snort.lua -r Fanetleader1-0-0-ethernet.pcap

sudo snort -c /etc/snort/snort.lua -r FanetleaderML-0-0-ethernet.pcap

#wait


# Step 2: Run Snort on Leader Node PCAP
snort -r fanetleaderML-0-0.pcap -A full -l ./snort_logs

# Step 3: Analyze PCAP with ML Model
python3 process_pcap_with_ml.py --pcap fanetleaderML-0-0.pcap

# Step 4: Combine Results (Optional)
echo "Snort and ML Analysis Completed."


# Check if the PCAP file exists
#if [ ! -f "Fanetleaderzeek-0-0-ethernet.pcap" ]; then
  #  echo "Error: Converted PCAP file not found. Exiting."
 #   exit 1
#fi

# Analyze the PCAP file with Zeek
#echo "Analyzing the PCAP file with Zeek..."
#zeek -r Fanetleaderzeek-0-0-ethernet.pcap

# Check if Zeek generated logs
#if [ ! -f "conn.log" ]; then
 #   echo "Error: Zeek log files not generated. Exiting."
  #  exit 1
#fi

# Parse Zeek logs (Optional step if using a Python ML pipeline)
#echo "Parsing Zeek logs for ML analysis (optional)..."
#python3 parse_zeek_logs.py conn.log http.log dns.log  # Replace with your Python script and required log files

# Wait for all processes to complete
wait

# Completion message
#echo "Simulation and Zeek analysis completed."
