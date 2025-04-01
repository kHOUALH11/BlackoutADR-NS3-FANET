import pandas as pd
import matplotlib.pyplot as plt

# Load the data - replace with actual paths to your CSV files
data_attack = pd.read_csv('fanet_data.csv')
data_no_attack = pd.read_csv('fanet_data_attack.csv')

# Plot RSSI values over time
plt.figure(figsize=(10, 6))
plt.plot(data_attack['Time'], data_attack['RSSI'], label='RSSI (With Attack)', color='red', linestyle='--')
plt.plot(data_no_attack['Time'], data_no_attack['RSSI'], label='RSSI (No Attack)', color='blue')
plt.xlabel('Time (s)')
plt.ylabel('RSSI (dBm)')
plt.title('RSSI Comparison With and Without Attack')
plt.legend()
plt.grid(True)
plt.show()

# Plot Transmission Power over time
plt.figure(figsize=(10, 6))
plt.plot(data_attack['Time'], data_attack['TransmissionPower'], label='Transmission Power (With Attack)', color='red', linestyle='--')
plt.plot(data_no_attack['Time'], data_no_attack['TransmissionPower'], label='Transmission Power (No Attack)', color='blue')
plt.xlabel('Time (s)')
plt.ylabel('Transmission Power (dBm)')
plt.title('Transmission Power Comparison With and Without Attack')
plt.legend()
plt.grid(True)
plt.show()

# Plot Packet Count over time
plt.figure(figsize=(10, 6))
plt.plot(data_attack['Time'], data_attack['PacketCount'], label='Packet Count (With Attack)', color='red', linestyle='--')
plt.plot(data_no_attack['Time'], data_no_attack['PacketCount'], label='Packet Count (No Attack)', color='blue')
plt.xlabel('Time (s)')
plt.ylabel('Packet Count')
plt.title('Packet Count Comparison With and Without Attack')
plt.legend()
plt.grid(True)
plt.show()
