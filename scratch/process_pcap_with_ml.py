#import pyshark
import numpy as np
import tensorflow as tf
import argparse

# Argument parser
parser = argparse.ArgumentParser()
parser.add_argument("--pcap", help="Path to the PCAP file to analyze", required=True)
args = parser.parse_args()

# Load the pretrained model
model = tf.keras.models.load_model("fnn_ids_model.h5")

# Feature extraction function
def extract_features_from_pcap(pcap_file):
    capture = pyshark.FileCapture(pcap_file)
    features = []
    for packet in capture:
        try:
            # Example features: Customize as per your training dataset
            features.append([
                int(packet.length),  # Packet length
                int(packet.ip.ttl),  # Time-to-live
                int(packet.ip.proto),  # Protocol (TCP/UDP)
                int(packet.transport_layer == "TCP"),  # Is TCP?
                int(packet.transport_layer == "UDP"),  # Is UDP?
            ])
        except AttributeError:
            # Skip packets that don't have the required fields
            continue
    capture.close()
    return np.array(features)

# Process PCAP file
features = extract_features_from_pcap(args.pcap)
if features.size > 0:
    predictions = model.predict(features)
    anomalies = np.where(predictions > 0.5)[0]  # Threshold of 0.5 for anomalies
    print(f"Anomalies detected in packets: {anomalies}")
else:
    print("No valid packets found in the PCAP.")
