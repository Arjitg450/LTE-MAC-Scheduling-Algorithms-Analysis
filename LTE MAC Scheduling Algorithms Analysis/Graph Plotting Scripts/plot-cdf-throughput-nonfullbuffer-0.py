import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read data from the CSV file
df = pd.read_csv("cdfNFB0.csv")

# Sort the data by the values in the second column (throughput)
df = df.sort_values(by=df.columns[1])

# Define the unique algorithms in the 'scheduler' column and corresponding colors
algorithms = df['Scheduler'].unique()
colors = ['b', 'g', 'r', 'c']  # You can customize these colors

# Create a 2x2 grid of subplots for each algorithm
fig, axs = plt.subplots(2, 2, figsize=(12, 8))

# Iterate through each algorithm, plot its CDF, and assign a color
for i, (algo, color) in enumerate(zip(algorithms, colors)):
    row = i // 2
    col = i % 2
    
    algo_data = df[df['Scheduler'] == algo]
    num_data_points = len(algo_data)
    cdf = np.arange(1, num_data_points + 1) / num_data_points
    
    axs[row, col].plot(algo_data.iloc[:, 1].to_numpy(), cdf, marker='.', linestyle='-', color=color, label=algo + "with 0m/s")
    axs[row, col].set_xlabel('Throughput')
    axs[row, col].set_ylabel('CDF')
    axs[row, col].set_title(f'Throughput CDF for {algo}')
    axs[row, col].legend()
    axs[row, col].grid(True)

# Remove any empty subplot
if len(algorithms) < 4:
    axs[-1, -1].axis('off')

plt.tight_layout()
plt.show()
