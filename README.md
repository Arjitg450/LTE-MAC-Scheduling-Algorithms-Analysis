# LTE-MAC-Scheduling-Algorithms-Analysis

Contributors: 

Arjit Gupta(cs23mtech12001)

Vikas Ravi Patil(cs22mtech12006)

LTE MAC Scheduling Algorithms Analysis using NS3 and graphs

## Instruction

After installing ns-3, you need to copy the cs22mtech12006-cs23mtech12001-asg1.cc into the scratch folder.

To run the code, use the command like this:

```
./ns3 run "<file-path> <args>"

```

where `args` are:

- `--speed`: speed of the ueNodes is 0 m/s or 5 m/s
- `--RngRun`: seed value for random position generation range from 1 to 5
- `--schType`: scheduler type (PF=0, RR=1, MT=2, BATS=3)
- `--fullBufferFlag`: use full buffer mode or not

For example:

```
./ns3 run "cs22mtech12006-cs23mtech12001-asg1 --speed=0 --RngRun=1 --schType=2 --fullBufferFlag=false"

```

## Trace Generation (All Data Traces) 
To generate all the Data traces, you need to run only one single python script. 

for Example: 

```
python3 auto-exe-whole-data-files.py 

```
This script will generate all the required traces including running lte traces of cs22mtech12006-cs23mtech12001-asg1.cc 

Note: If you run this command then no need to run 'ns3' command to execute cs22mtech12006-cs23mtech12001-asg1.cc script.

## Data Collection for plot

To collect data from traces, you need to just run one python script and it will collect all data required for plotting.

for example: 
```
python3 thoughputGenerator.py

```
This script will generate the required data collection for graphs as asked in the assignment. 

## Graph Plotting --> 

To plot the graphs there are python scripts which used to plot various graphs as mentioned: 

The scripts are:  

- `plot-avg-throughput-fullbufffer.py`: plot average throughput full buffer at different speeds
- `plot-avg-throughput-nonfullbuffer.py`: plot average throughput full buffer at 0 m/s speed
- `plot-cdf-throughput-fullbuffer-0.py`: plot CDF vs throughput graph at 0 m/s and fullbuffer case
- `plot-cdf-throughput-fullbuffer-5.py`: plot CDF vs throughput graph at 10 m/s and fullbuffer case
- `plot-cdf-throughput-nonfullbuffer-0.py`: plot CDF vs throughput graph at 0 m/s and nonfullbuffer case
- `plot-cdf-throughput-nonfullbuffer-5.py`: plot CDF vs throughput graph at 5 m/s and nonfullbuffer case
- `time_vs_throughput.py.py`: plot SINR and throughput for ue 0 with time at speed 0 m/s
- `time_vs_throughput.py.py`: plot SINR and throughput for ue 0 with time at speed 5 m/s

### REM Plot

Note: For plotting rem, first make one change in the cs22mtech12006-cs23mtech12001-asg1.cc file 
at line no. 360 make it 'true' then execute following command. 
```
./ns3 run cs22mtech12006-cs23mtech12001-asg1.cc

```
After creating the `cs22mtech12006-cs23mtech12001-asg1.out` file, run the following command in terminal:

```
gnuplot
set terminal png
set output 'output_image.png'
set view map;
set xlabel "X"
set ylabel "Y"
set cblabel "SINR (dB)"
unset key
plot "cs22mtech12006-cs23mtech12001-asg1.out" using ($1):($2):(10*log10($4)) with image
```

