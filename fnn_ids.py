import tensorflow as tf
import os
import matplotlib.pyplot as plt
from keras.models import Sequential
from keras.layers import Dense
import pandas as pd
import numpy as np
import pyshark  # For PCAP file parsing
from sklearn.model_selection import train_test_split
import argparse

# Argument parser
parser = argparse.ArgumentParser()
parser.add_argument("--input", help="Path to PCAP file", required=True)
parser.add_argument("--fusion", help="choose fusion")
parser.add_argument("--output", help="Output results CSV file", required=True)
parser.add_argument("--title_name", help="Print title name")
args = parser.parse_args()

# Define feature extraction function
def extract_features_from_pcap(pcap_file):
    """Extract features from PCAP file for FNN model."""
    print(f"Processing PCAP file: {pcap_file}")
    cap = pyshark.FileCapture(pcap_file)
    extracted_data = []

    for packet in cap:
        try:
            # Extract basic features
            frame_len = int(packet.length)
            protocol = packet.highest_layer if hasattr(packet, "highest_layer") else "UNKNOWN"
            src_ip = packet.ip.src if "IP" in protocol else "0.0.0.0"
            dst_ip = packet.ip.dst if "IP" in protocol else "0.0.0.0"
            src_port = packet[packet.transport_layer].srcport if hasattr(packet, "transport_layer") else 0
            dst_port = packet[packet.transport_layer].dstport if hasattr(packet, "transport_layer") else 0

            # Add extracted features to list
            extracted_data.append([
                frame_len, protocol, src_ip, dst_ip, src_port, dst_port
            ])
        except AttributeError:
            # Handle packets without expected attributes
            pass

    cap.close()

    # Convert to Pandas DataFrame
    df = pd.DataFrame(extracted_data, columns=[
        "frame.len", "protocol", "src_ip", "dst_ip", "src_port", "dst_port"
    ])
    return df

# Define FNN model creation
def create_model(input_dim, neurons=128, activation='relu', hidden_layers=3):
    """Create a feedforward neural network."""
    model = Sequential()
    model.add(Dense(neurons, input_dim=input_dim, activation=activation))

    for _ in range(hidden_layers - 1):
        model.add(Dense(neurons, activation=activation))

    model.add(Dense(1, activation='sigmoid'))  # Binary classification
    model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
    return model

# Process PCAP file
pcap_file = args.input
features = extract_features_from_pcap(pcap_file)

# Dummy preprocessing for the extracted features
# Assuming you already know how to preprocess the extracted features
print("Preprocessing features...")
features["is_anomaly"] = 0  # Placeholder target column
X = features.drop(columns=["is_anomaly"]).select_dtypes(include=[np.number]).values
y = features["is_anomaly"].values

# Split dataset
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Train FNN
print("Training FNN model...")
model = create_model(input_dim=X_train.shape[1])
history = model.fit(X_train, y_train, epochs=20, batch_size=32, validation_split=0.2)

# Evaluate the model
print("Evaluating model...")
loss, accuracy = model.evaluate(X_test, y_test)
print(f"Test Loss: {loss}, Test Accuracy: {accuracy}")

# Predict anomalies
predictions = model.predict(X_test)
features["predictions"] = (predictions > 0.5).astype(int)

# Save results
print(f"Saving results to {args.output}...")
features.to_csv(args.output, index=False)
print("Analysis complete.")

# Plot learning curves
plt.plot(history.history['accuracy'], label='Accuracy')
plt.plot(history.history['val_accuracy'], label='Validation Accuracy')
plt.xlabel('Epoch')
plt.ylabel('Accuracy')
plt.title('Training and Validation Accuracy')
plt.legend()
plt.savefig("accuracy_plot.png")
