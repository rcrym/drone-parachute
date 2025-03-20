import pandas as pd
import matplotlib.pyplot as plt

# Define file path
file_path = 'drop-test-data/test-jan-28-num-5-elevator-mode-pressure-and-velocity.csv'

# Read the CSV file
data = pd.read_csv(file_path)

# Plot the data
plt.figure(figsize=(12, 10))

# Plot Pressure vs Time
plt.subplot(3, 1, 1)
plt.plot(data['Time (s)'], data['Pressure (hPa)'], label='Pressure (hPa)', color='blue')
plt.title('Pressure vs Time')
plt.xlabel('Time (s)')
plt.ylabel('Pressure (hPa)')
plt.legend()

# Plot Altitude vs Time
plt.subplot(3, 1, 2)
plt.plot(data['Time (s)'], data['Altitude (m)'], label='Altitude (m)', color='green')
plt.title('Altitude vs Time')
plt.xlabel('Time (s)')
plt.ylabel('Altitude (m)')
plt.legend()

# Plot Velocity vs Time (using Time (velocity))
plt.subplot(3, 1, 3)
plt.plot(data['Time (velocity) (s)'], data['Velocity (m/s)'], label='Velocity (m/s)', color='red')
plt.title('Velocity vs Time')
plt.xlabel('Time (s)')
plt.ylabel('Velocity (m/s)')
plt.legend()

# Adjust layout
plt.tight_layout()
plt.show()
