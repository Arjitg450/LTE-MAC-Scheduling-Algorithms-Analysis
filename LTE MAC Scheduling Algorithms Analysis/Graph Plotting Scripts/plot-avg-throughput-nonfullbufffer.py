#Avg Throughput Ploter
# Import the libraries
import pandas as pd
import matplotlib.pyplot as plt

# Read the data from the CSV file
df = pd.read_csv('throughputWithoutFullBuffer.csv', header=None, names=['Algorithm', 'Speed', 'AvgThroughput'])

# Group the data by 'Speed' and 'Algorithm' and calculate the mean AvgThroughput for each group
grouped_data = df.groupby(['Speed', 'Algorithm'])['AvgThroughput'].mean().unstack()

# Plot a bar chart using matplotlib
ax = grouped_data.plot(kind='bar', width=0.8, figsize=(10, 6))

# Set labels and title
plt.xlabel('Speed(m/s)')
plt.ylabel('AvgThroughput(bps)')
plt.title('Bar chart of Avg Throughput vs speed with subcategories for algorithms')

# Add a legend
plt.legend(title='Algorithm', loc='upper right')

# Show the plot
plt.show()
