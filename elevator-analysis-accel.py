import pandas as pd
import matplotlib.pyplot as plt

# Define file path
file_path = 'drop-test-data/Kaiser 3 Acceleration.csv'

# Read the CSV file
data = pd.read_csv(file_path)

# Plot the data
plt.figure(figsize=(10, 6))

# Plot Acceleration vs Time
plt.plot(data['Time (s)'], data['Acceleration (m/s²)'], label='Acceleration (m/s²)', color='blue')
plt.title('Acceleration vs Time')
plt.xlabel('Time (s)')
plt.ylabel('Acceleration (m/s²)')
plt.grid(True)
plt.legend()

# Show the plot
plt.tight_layout()
plt.show()
