import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.colors as mcolors

# List of provided files
files = [
    "drop-test-data/Kaiser 1 Pressure and velocity.csv",
    "drop-test-data/Kaiser 3 Pressure and velocity.csv",
    "drop-test-data/Kaiser 5 Pressure and velocity.csv",
    "drop-test-data/Kaiser 7 Pressure and velocity.csv"
]

time_offsets = [121, 0, 133, 78]

# Function to generate a gradient of colors
def get_hue_variation(start_hue, end_hue, index, total_files, brightness=0.8, saturation=0.9):
    hue_range = np.linspace(start_hue, end_hue, total_files)
    new_hue = hue_range[index] % 1
    return mcolors.hsv_to_rgb((new_hue, saturation, brightness))

# Define hue ranges
cool_start, cool_end = 0.5, 0.7
warm_start, warm_end = 0.0, 0.1

plt.figure(figsize=(12, 8))

# Plot Altitude vs Time
plt.subplot(2, 1, 1)
for i, file in enumerate(files):
    df = pd.read_csv(file)
    time_altitude = df.iloc[:, 0] + time_offsets[i]
    altitude = df.iloc[:, 2]
    mask = (time_altitude >= 295) & (time_altitude <= 330)
    plt.plot(time_altitude[mask], altitude[mask], label=f'Test {i+1}', 
             color=get_hue_variation(cool_start, cool_end, i, len(files)),
             linewidth=2, alpha=0.4)
    # Add black dots to data points
    plt.scatter(time_altitude[mask], altitude[mask], color='black', alpha=1, s=10)

plt.title('Altitude vs Time')
plt.xlabel('Time (s)')
plt.ylabel('Altitude (m)')
plt.legend()

# Plot Velocity vs Time
plt.subplot(2, 1, 2)
for i, file in enumerate(files):
    df = pd.read_csv(file)
    time_velocity = df.iloc[:, 3] + time_offsets[i]
    velocity = df.iloc[:, 4]
    mask = (time_velocity >= 295) & (time_velocity <= 330)
    color = get_hue_variation(warm_start, warm_end, i, len(files))
    plt.plot(time_velocity[mask], velocity[mask], label=f'Test {i+1}', color=color, linewidth=2, alpha=0.4)
    # Add black dots to data points
    plt.scatter(time_velocity[mask], velocity[mask], color='black', alpha=1, s=10)

    # Add minimum velocity line and text
    filtered_time_velocity = time_velocity[mask]
    filtered_velocity = velocity[mask]
    if not filtered_velocity.empty:
        min_velocity = filtered_velocity.min()
        min_velocity_time = filtered_time_velocity[filtered_velocity.idxmin()]
        plt.axhline(y=min_velocity, color="black", linestyle='dashed', linewidth=1)
        plt.text(min_velocity_time + 10, min_velocity*0.8 - (0.25 * i) + 0.7, f"Max abs: {np.abs(min_velocity):.2f} m/s", 
                 verticalalignment='bottom', horizontalalignment='left', fontsize=10, color="black")

plt.title('Velocity vs Time')
plt.xlabel('Time (s)')
plt.ylabel('Velocity (m/s)')
plt.legend()

# Adjust layout
plt.tight_layout()
plt.show()
