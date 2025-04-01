#!/bin/bash
# Run ns-3 simulation

# Run the FANET simulation
echo "Starting FANET simulation..."

#./ns3 run scratch/CSVIDSleaderFANET3D.cc &

#./ns3 run scratch/FANET3DIDSNoattack.cc &
./ns3 run scratch/FANET3DIDSWithattack.cc 
#&


# Give the FANET simulation some time to start (we adjust the sleep time based on our needs)
#sleep 10

# Run the attacker script
echo "Launching attack on the leader node..."
#python3 attackscript/ attacker.py &


# Run Snort (starting Snort command)
echo "Starting Snort..."
#snort -i wlan0 -A console -c /etc/snort/snort.lua &

# Convert to Ethernet format
#editcap -T ether Fanet3D-leader-newfile-0-0.pcap Fanet3D-leader-newfile-0-0-ethernet2.pcap

#run snort on the converted ether  PCAP file 

#sudo snort -c /etc/snort/snort.lua -r /home/hidawi/ns-allinone-3.42/ns-3.42/Fanet3D-leader-newfile-0-0-ethernet2.pcap


# Wait for both processes to complete (Simulation and Attack run for 60 seconds)
#wait

#echo "Simulation and attack completed."

#./ns3 run scratch/IDSleaderFANET3D.cc

# Convert to Ethernet format
#editcap -T ether Fanetleader1-0-0.pcap Fanetleader1-0-0-ethernet.pcap

editcap -T ether Fanetleader2-0-0.pcap Fanetleader2-0-0-ethernet.pcap

# Run Snort on the converted pcap file
#sudo snort -c /etc/snort/snort.lua -r Fanetleader1-0-0-ethernet.pcap

sudo snort -c /etc/snort/snort.lua -r Fanetleader2-0-0-ethernet.pcap

wait

