import pandas as pd
import matplotlib.pyplot as plt

# Since the CSV file doesn't have headers, we define them based on the data structure
headers = ['Time', 'Scheduler', 'Throughput']

# Load the CSV file into a DataFrame with the headers
df = pd.read_csv('Ue0FB5seed1.csv', names=headers)

# Assign colors to each scheduler
colors = {'RR': 'green', 'MT': 'blue', 'BATS': 'cyan', 'PF': 'red'}

# Set up the matplotlib figure and axes
plt.figure(figsize=(14, 7))

# Plot the data for each scheduler with the specified color
for scheduler in df['Scheduler'].unique():
    scheduler_data = df[df['Scheduler'] == scheduler]
    plt.plot(scheduler_data['Time'].to_numpy(), scheduler_data['Throughput'].to_numpy(), label=scheduler, color=colors.get(scheduler, 'black'))

# Adding title and labels
plt.title('Time vs. Throughput for Different Schedulers')
plt.xlabel('Time (ms)')
plt.ylabel('Throughput')

# Adding a legend
plt.legend()

# Display the plot
plt.show()

