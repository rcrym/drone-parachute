import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Replace 'your_file.csv' with the path to your CSV file
file_path = 'drop-test-data/Kaiser 7 Acceleration.csv'

# Reading the CSV file
data = pd.read_csv(file_path)

# Define the time range for filtering data
t_start = 229
t_end = 234

# Filter the data based on the time range
filtered_data = data[(data['Time (s)'] >= t_start) & (data['Time (s)'] <= t_end)].copy()

# Remove gravitational acceleration from z component to get acceleration relative to free fall
filtered_data['Linear Acceleration z (m/s^2)'] = filtered_data['Acceleration (m/s²)'];

# Convert initial height from feet to meters (36 ft -> 10.9728 m)
initial_height = 20  # meters
initial_velocity = 0  # m/s (assumed)

# Compute time differences correctly
filtered_data['Time Diff'] = np.diff(filtered_data['Time (s)'], prepend=filtered_data['Time (s)'].iloc[0])
filtered_data.iloc[0, filtered_data.columns.get_loc('Time Diff')] = 0  # Ensure first element is 0

print(filtered_data['Time Diff'])
# Compute velocity by integrating acceleration (trapezoidal rule)
filtered_data['Velocity z'] = initial_velocity + np.cumsum(filtered_data['Linear Acceleration z (m/s^2)'] * filtered_data['Time Diff'])

# Compute height by integrating velocity (trapezoidal rule)
filtered_data['Height'] = initial_height + np.cumsum(filtered_data['Velocity z'] * filtered_data['Time Diff'])

# Plotting the data
plt.figure(figsize=(12, 8))

# Plot acceleration
plt.subplot(3, 1, 1)
# plt.plot(filtered_data['Time (s)'], filtered_data['Linear Acceleration x (m/s^2)'], label='Acceleration x', color='r')
# plt.plot(filtered_data['Time (s)'], filtered_data['Linear Acceleration y (m/s^2)'], label='Acceleration y', color='g')
plt.plot(filtered_data['Time (s)'], filtered_data['Linear Acceleration z (m/s^2)'], label='Acceleration z', color='b')
plt.title(f'Linear Accelerations vs Time ({t_start}s to {t_end}s)')
plt.xlabel('Time (s)')
plt.ylabel('Acceleration (m/s²)')
plt.legend()

# Plot absolute acceleration
# plt.subplot(3, 1, 2)
# plt.plot(filtered_data['Time (s)'], filtered_data['Absolute acceleration (m/s^2)'], label='Absolute Acceleration', color='purple')
# plt.title(f'Absolute Acceleration vs Time ({t_start}s to {t_end}s)')
# plt.xlabel('Time (s)')
# plt.ylabel('Acceleration (m/s²)')
# plt.legend()

# Plot velocity and height
plt.subplot(3, 1, 3)
plt.plot(filtered_data['Time (s)'], filtered_data['Velocity z'], label='Velocity (m/s)', color='orange')
plt.plot(filtered_data['Time (s)'], filtered_data['Height'], label='Height (m)', color='blue')
plt.title(f'Velocity and Height vs Time ({t_start}s to {t_end}s)')
plt.xlabel('Time (s)')
plt.ylabel('Velocity (m/s) & Height (m)')
plt.legend()

# Adjust layout
plt.tight_layout()
plt.show()
