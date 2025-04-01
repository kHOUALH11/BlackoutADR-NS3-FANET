
```markdown
# BlackoutADR: Exploiting ADR Vulnerabilities in LoRaWAN-based FANETs

## Overview
This repository contains the implementation of **BlackoutADR**, an adversarial attack targeting the Adaptive Data Rate (ADR) mechanism in LoRaWAN-based Flying Ad Hoc Networks (FANETs). The project simulates a FANET with 20 UAVs using NS-3, where the leader node (Node 0) acts as the central gateway equipped with Intrusion Detection Systems (IDS). BlackoutADR manipulates signal metrics (RSSI/SNR) to trick UAVs into increasing transmission power, causing power drain and potential DDoS attacks. The simulation evaluates IDS effectiveness using Snort, a LoRaWAN-specific IDS (KLD/HD algorithms), and machine learning (ML)-based IDS models, with findings showing evasion by BlackoutADR’s subtle manipulations.

## Key Features
- **FANET Setup**: 20 UAV nodes, with Node 0 as the backbone UAV (gateway) in a star-of-stars topology.
- **LoRaWAN Protocol**: Implements LoRaWAN with ADR, vulnerable to signal manipulation.
- **Attack**: BlackoutADR targets ADR to increase power consumption via crafted packets and delayed ACKs.
- **IDS Evaluation**:
  - **Snort**: Signature-based IDS with custom rules for packet analysis.
  - **LoRaWAN-based IDS**: Uses KLD and HD algorithms to detect join procedure anomalies.
  - **ML-based IDS**: Classifies traffic using features like packet size and RSSI for advanced detection.
- **Simulation**: NS-3 with NetAnim visualization, Kali Linux tools (e.g., Scapy, Wireshark) for attack and analysis.

## Repository Structure

### Main Directory:
- `run_IDS_fanet_with_snort.sh`: Shell script to automate the NS-3 simulation, Snort IDS, and attack execution.
- `fanet_IDS_zeek.sh`: Script to run Zeek-based IDS analysis on FANET traffic.
- `attacker.py`: Example Python script to simulate BlackoutADR by manipulating ADR (e.g., packet delays, payload sizes).
- `graphs.py`: Script to generate visualization graphs (e.g., RSSI, power over time).
- `utils.py`: Utility functions for data processing and simulation.
- `Fanet3DLoraWan.xml, Fanet3DLoraWanleader.xml`: NetAnim XML files for visualizing the FANET simulation.
- `Fanet3D-*.pcap, Fanetleader*.pcap`: PCAP files capturing FANET traffic for nodes 0-19 and leader node.
- `Fanet3D.tr`: Trace file for NS-3 simulation.
- `fanet_data*.csv`: CSV files with processed FANET data (e.g., with/without attack, Zeek analysis).
- `Dataset_T-ITS.csv`: Dataset used in the study, sourced from IEEE T-ITS.
- `implemnationreport.pdf`: Implementation report detailing the project setup and findings.
- `CMakeLists.txt, CONTRIBUTING.md, LICENSE, README.md`: Standard repository files.

### Subdirectories:
- **Datasets_processed/**: Contains processed CSV datasets (e.g., traffic features, attack data). Note: Not all datasets are included due to large file sizes; contact the repository owner for access to full datasets.
- **H5_trained_models/**: Contains trained ML-IDS models:
  - `bilstm_ids_model.h5`: BiLSTM model for traffic classification.
  - `cnn_ids_model.h5`: CNN model for traffic classification.
  - `fnn_ids_model.h5`: FNN model for traffic classification.
  - `lorawan_ids_model.h5`: Model for LoRaWAN-specific anomaly detection.
  - `lstm_ids_model.h5`: LSTM model for traffic classification.
- **attackscript/**: Contains attack-related scripts (e.g., attacker.py duplicate, additional attack utilities).
- **scratch/**: NS-3 simulation scripts to be placed in the NS-3 scratch/ directory for running the FANET simulation.

## Prerequisites
- **Operating System**: Linux (e.g., Ubuntu 20.04 recommended; Windows users can use WSL).
- **NS-3**: Version 3.42, with LoRaWAN module support.
- **NetAnim**: For visualizing the FANET simulation.
- **Snort**: Version 3.1.82.0, for IDS deployment.
- **Zeek**: For additional IDS analysis.
- **Kali Linux Tools**: Wireshark, Scapy for vulnerability assessment.
- **Python**: Version 3.8+, with libraries:
  - `scapy` (for packet crafting)
  - `PyNs3` (for ML-IDS integration)
  - `tensorflow` (for ML models)

### Dependencies:
```bash
sudo apt-get update
sudo apt-get install build-essential python3 python3-pip wireshark
pip3 install scapy tensorflow
```

### Snort Installation:
```bash
sudo apt-get install snort
```

### NetAnim Installation:
```bash
git clone https://github.com/nsnam/netanim.git
cd netanim
make
```

## Setup Instructions

### Clone the Repository:
```bash
git clone https://github.com/kHOUALH11/BlackoutADR-NS3-FANET.git
cd BlackoutADR-NS3-FANET
```

### Configure NS-3:
- Install NS-3.42 and ensure the LoRaWAN module is enabled.
- Copy the scratch/ folder to your NS-3 scratch/ directory:
  ```bash
  cp -r scratch/ /path/to/ns-3.42/scratch/
  ```

### Set Up ML Models:
- Place the .h5 models from `H5_trained_models/` in your working directory for ML-IDS usage.

### Prepare Scripts:
- Ensure `run_IDS_fanet_with_snort.sh`, `fanet_IDS_zeek.sh`, and `attacker.py` are executable:
  ```bash
  chmod +x run_IDS_fanet_with_snort.sh fanet_IDS_zeek.sh attacker.py
  ```

## Running the Simulation

### 1. Start the FANET Simulation with Snort IDS
The simulation runs a FANET with 20 UAVs, with Node 0 (backbone UAV) as the gateway running Snort for IDS. The `run_IDS_fanet_with_snort.sh` script automates the simulation, attack, and Snort monitoring.

#### Edit the Script (if needed):
```bash
nano run_IDS_fanet_with_snort.sh
```
Example content:
```bash
#!/bin/bash
# Start NS-3 simulation
./waf --run scratch/fanet_simulation &
sleep 5  # Wait for simulation to start
# Launch attacker script (example)
python3 attacker.py &
# Run Snort in real-time
sudo snort -i wlan0 -A console -c /etc/snort/snort.lua &
# Convert wireless packets to Ethernet format for offline analysis
editcap -T ether Fanet3D-leader-n-0-0.pcap Fanet3D-leader-n-0-0-ethernet.pcap
# Run Snort offline on pcap
sudo snort -c /etc/snort/snort.lua -r Fanet3D-leader-n-0-0-ethernet.pcap
```

#### Run the Simulation:
```bash
./run_IDS_fanet_with_snort.sh
```

### 2. Visualize the FANET Simulation with NetAnim
NetAnim visualizes the 20 UAVs, with Node 0 (backbone UAV) in green.
#### Run NetAnim:
```bash
./NetAnim
```
- Open `Fanet3DLoraWan.xml` or `Fanet3DLoraWanleader.xml` in NetAnim to view the topology and UAV movements.

### 3. Analyze Traffic with Zeek
Run Zeek-based IDS:
```bash
./fanet_IDS_zeek.sh
```
Processes PCAP files (e.g., `Fanetleaderzeek-*.pcap`) and outputs analysis (e.g., `fanet_data_with_attack_zeek.csv`).

### 4. Run the Attack Script (Example)
Execute BlackoutADR Attack (Example):
```bash
python3 attacker.py
```
This is an example script that targets Node 0 (192.168.1.1, port 9), sending packets with random sizes (50-1024 bytes) and delays (0.5-2.0 seconds).

### 5. Run ML-based IDS (Optional)
Execute ML-IDS:
```bash
python3 fnn_ids.py  # Example for FNN model
```
Uses models from `H5_trained_models/` (e.g., `fnn_ids_model.h5`) to classify traffic in real-time.

## Implementation Details

### FANET Setup
- **Topology**: Star-of-stars with 20 UAVs; Node 0 (backbone UAV) as the gateway, others as end nodes.
- **LoRaWAN**: Implements LoRaWAN with ADR, adjusting data rate and power based on RSSI/SNR.

### NS-3 Configuration:
- **Node 0**: `UdpEchoServer` for receiving packets, `WifiRemoteStationManager` for ADR adjustments.
- **Other nodes**: `UdpEchoClient` for sending packets, adjusting power based on RSSI.

### BlackoutADR Attack
**Objective**: Exploit ADR to cause power drain by increasing transmission power.

**Methods**:
- Manipulate RSSI/SNR via simulated noise in NS-3.
- Fake network conditions with crafted packets.
- Delay ACKs to force retransmissions at higher power.

### IDS Approaches
#### Snort IDS:
- Deployed on Node 0, monitors traffic in real-time and offline via PCAP files.
- Custom rules detect UDP floods and large packets but fails to detect BlackoutADR’s subtle patterns.

#### LoRaWAN-based IDS:
- Targets LoRaWAN join procedure anomalies using KLD and HD algorithms on Node 0.

#### ML-based IDS:
- Uses models in `H5_trained_models/` to classify traffic in real-time using PyNs3.

## Findings
- **ADR Response**: Node 0 increases power to 20 dBm when RSSI drops below -85 dBm.
- **Snort Detection**: Fails to flag BlackoutADR due to gradual manipulations.
- **LoRaWAN IDS**: KLD outperforms HD but misses subtle attacks.
- **ML-IDS**: Shows potential for detecting complex patterns, ongoing evaluation.

