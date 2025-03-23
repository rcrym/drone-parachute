import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import subprocess
import os

# === INPUTS ===
csv_path = "drop-test-data/drone-mar-21-test-with-fall-detected.csv"
video_path = "drone.mp4"  # 👈 change this to your actual video path
output_path = "animated_drone_graph.mp4"
final_combined_output = "drone_and_graph_combined.mp4"

# === LOAD DATA ===
try:
    df = pd.read_csv(csv_path)
except FileNotFoundError:
    raise FileNotFoundError(f"CSV file not found at: {csv_path}")

df = df[(df['Time'] >= 405.53) & (df['Time'] <= 413)].reset_index(drop=True)

# === CREATE GRAPH ANIMATION ===
fig, ax = plt.subplots(figsize=(12, 6))

# Faded background lines
ax.plot(df['Time'], df['AX'], color='lightblue', linewidth=1, alpha=0.3)
ax.plot(df['Time'], df['AY'], color='orange', linewidth=1, alpha=0.3)
ax.plot(df['Time'], df['AZ'], color='green', linewidth=1, alpha=0.3)
ax.plot(df['Time'], df['|A|'], color='red', linewidth=1, alpha=0.3)

# Live animated lines
line_ax, = ax.plot([], [], label='AX', color='lightblue', linewidth=1.5)
line_ay, = ax.plot([], [], label='AY', color='orange', linewidth=1.5)
line_az, = ax.plot([], [], label='AZ', color='green', linewidth=1.5)
line_mag, = ax.plot([], [], label='|A|', color='red', linewidth=1.5)

# Annotations
ax.axvline(x=408.320, color='deepskyblue', linestyle='--', linewidth=1.5, label='Cut Motors')
ax.axvline(x=410.333, color='peru', linestyle='--', linewidth=1.5, label='Hit the ground')
ax.axhspan(ymin=df[['AX', 'AY', 'AZ']].min().min(), ymax=1, color='red', alpha=0.2, label='|A| Trigger Threshold')

# Fall detection shading
in_fall = False
start_time = None
for i in range(len(df)):
    if df.loc[i, 'FallDetected'] == 1 and not in_fall:
        in_fall = True
        start_time = df.loc[i, 'Time']
    elif df.loc[i, 'FallDetected'] == 0 and in_fall:
        end_time = df.loc[i, 'Time']
        ax.axvspan(start_time, end_time, color='green', alpha=0.2, label='Fall Detected')
        in_fall = False
if in_fall:
    ax.axvspan(start_time, df.loc[len(df)-1, 'Time'], color='green', alpha=0.2, label='Fall Detected')

# Final touches
handles, labels = ax.get_legend_handles_labels()
unique = dict(zip(labels, handles))
ax.legend(unique.values(), unique.keys())
ax.set_xlim(df['Time'].min(), df['Time'].max())
ax.set_ylim(df[['AX', 'AY', 'AZ', '|A|']].min().min() - 1, df[['AX', 'AY', 'AZ', '|A|']].max().max() + 1)
ax.set_xlabel('Time (s)')
ax.set_ylabel('Acceleration (m/s²)')
ax.set_title('Accelerometer Data - Totem Field, March 21st')
ax.grid(True)
plt.tight_layout()

# Animation update
def update(frame):
    line_ax.set_data(df['Time'][:frame], df['AX'][:frame])
    line_ay.set_data(df['Time'][:frame], df['AY'][:frame])
    line_az.set_data(df['Time'][:frame], df['AZ'][:frame])
    line_mag.set_data(df['Time'][:frame], df['|A|'][:frame])
    return line_ax, line_ay, line_az, line_mag

# Save animation
ani = animation.FuncAnimation(fig, update, frames=len(df), interval=50, blit=True)
ani.save(output_path, writer='ffmpeg', fps=20)
plt.close()

# === COMBINE WITH VIDEO ===
if not os.path.exists(video_path):
    raise FileNotFoundError(f"Drone video not found at: {video_path}")

# Optional: resize to same height first
combine_cmd = [
    "ffmpeg", "-y",
    "-t", "5.7",                  # Trim drone video only
    "-i", video_path,             # Input 0 = drone video (gets trimmed)
    "-i", output_path,            # Input 1 = animated graph
    "-filter_complex",
    "[1:v]scale=-1:720,setsar=1[v0];"    # Graph on the left
    "[0:v]scale=-1:720,setsar=1[v1];"  # Drone on the right (rotated)
    "[v0][v1]hstack=inputs=2[v];"
    "[v]pad=ceil(iw/2)*2:ceil(ih/2)*2[vout]",
    "-map", "[vout]",
    "-map", "0:a?",
    "-c:v", "libx264",
    "-r", "30",
    "-crf", "23",
    "-preset", "veryfast",
    "-movflags", "+faststart",
    final_combined_output
]


subprocess.run(combine_cmd, check=True)

print(f"✅ Combined video saved to: {final_combined_output}")
