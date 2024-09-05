import subprocess
from multiprocessing import Pool

# Define the values to iterate over
speed_values = [0,5]
schType_values = [0,1,2,3]
RngRun_values = [1,2,3,4,5]
fullBufferFlag_values = [True, False]

# Define the command template
command_template = "./ns3 run 'scratch/asg1 --speed={} --RngRun={} --schType={} --fullBufferFlag={}'"

# Function to execute the command
def run_simulation(speed, schType, RngRun, fullBufferFlag):
    # Create the command with specific values
    command = command_template.format(speed, RngRun, schType, fullBufferFlag)

    # Run the command
    subprocess.call(command, shell=True)

# Create a pool of workers to run tasks in parallel
if __name__ == "__main__":
    with Pool(processes=4) as pool:  # Adjust the number of processes as needed
        for speed in speed_values:
            for schType in schType_values:
                for RngRun in RngRun_values:
                    for fullBufferFlag in fullBufferFlag_values:
                        pool.apply_async(run_simulation, (speed, schType, RngRun, fullBufferFlag))
        pool.close()
        pool.join()
