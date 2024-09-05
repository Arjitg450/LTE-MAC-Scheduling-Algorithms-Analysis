import os
import pandas as pd

#write loop to read all files in a folder and extract data from all files and save new files with the same name 

# Function to extract and return data from the file
def extract_columns(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
    
    # Assuming the first line contains column headers
    headers = lines[0].strip().split('\t')
    
    # Find indices for "start", "end", and "RxBytes" columns
    start_index = headers.index('% start')
    end_index = headers.index('end')
    CellId_index=headers.index('CellId')
    IMSI_index=headers.index('IMSI')
    RNTI_index=headers.index('RNTI')
    LCID_index=headers.index('LCID')
    nTxPDUs_index=headers.index('nTxPDUs')
    TxBytes_index=headers.index('TxBytes')
    nRxPDUs_index=headers.index('nRxPDUs')
    rxbytes_index = headers.index('RxBytes')
    delay_index=headers.index('delay')
    stdDev_index=headers.index('stdDev')
    min_index=headers.index('min')
    max_index=headers.index('max')
    PduSize_index=headers.index('PduSize')
    stdDev_index1=headers.index('stdDev')
    min_index1=headers.index('min')
    max_index1=headers.index('max')

    # Extract values for the specified columns
    data = []
    for line in lines[1:]:  # Skipping header line
        values = line.strip().split('\t')
        start = values[start_index]
        end = values[end_index]
        rxbytes = values[rxbytes_index]
        CellId = values[CellId_index]
        IMSI = values[IMSI_index]
        RNTI = values[RNTI_index]
        LCID = values[LCID_index]
        nTxPDUs = values[nTxPDUs_index]
        TxBytes = values[TxBytes_index]
        nRxPDUs = values[nRxPDUs_index]
        delay = values[delay_index]
        stdDev = values[stdDev_index]
        min = values[min_index]
        max = values[max_index]
        PduSize = values[PduSize_index]
        stdDev1 = values[stdDev_index1]
        min1 = values[min_index1]
        max1 = values[max_index1]
        data.append((start, end, CellId, IMSI, RNTI, LCID, nTxPDUs, TxBytes, nRxPDUs, rxbytes, delay, stdDev, min, max, PduSize, stdDev1, min1, max1))
        
    return data


log_dir = "../Data"

log_files = os.listdir(log_dir)

for log_file in log_files:
    
    if log_file.startswith('DlRlcStats'):

        #split the name of the file as sceduler name, speed, buffer size, and seed after DlRlcStats
        scheduler, speed, buffermod, seed = log_file.split('-')[1:]
        buffermod = int(buffermod)
        speed = int(speed)
        seed = int(seed.split('.')[0])
        # print(f"Buffermod: {buffermod}")
        # Extract data from the file
        extracted_data = extract_columns(os.path.join(log_dir, log_file))

        # Convert extracted data to DataFrame
        df = pd.DataFrame(extracted_data, columns=['start', 'end', 'CellId', 'IMSI', 'RNTI', 'LCID', 'nTxPDUs', 'TxBytes', 'nRxPDUs', 'RxBytes', 'delay', 'stdDev', 'min', 'max', 'PduSize', 'stdDev1', 'min1', 'max1'])

        # Convert columns to numeric values
        df['start'] = pd.to_numeric(df['start'])
        df['end'] = pd.to_numeric(df['end'])
        df['RxBytes'] = pd.to_numeric(df['RxBytes'])
        # Perform the calculation RxBytes * 8 / (end - start) for each row
        df['Throughput'] = df['RxBytes'] * 8 / (df['end'] - df['start'])/1e6

        throughput = df['Throughput'].mean()

        print(f"Average throughput for {log_file} is {throughput} Mbps")

        if buffermod == 1:
            #open csv file and write the data to the file as scheduler, speed, throughput
            with open('throughputWithFullBuffer.csv', 'a') as f:
                f.write(f"{scheduler},{speed},{throughput}\n")
                print(f"Results have been saved to throughputWithFullBuffer.csv")

            if speed == 0: 
                #open csv file and write the data to the file as scheduler, speed, throughput
                with open('cdfFB0.csv', 'a') as f:
                    #write whole df[thruput] to the file 
                    for i in df['Throughput']:
                        f.write(f"{scheduler},{i}\n")
                    print(f"Results have been saved to cdfFB0.csv")
                
                if seed == 1:
                    #open csv file and write the data to the file as scheduler, speed, throughput
                     with open('Ue0FB0seed1.csv', 'a') as f:
                        #write whole df[thruput] to the file based on IMSI value == 1   
                        for i, j in zip(df[df['IMSI'] == '1']['Throughput'], df[df['IMSI'] == '1']['end']):
                            #write end time for specific IMSI value 
                            f.write(f"{j},{scheduler},{i}\n")

            if speed == 5:
                #open csv file and write the data to the file as scheduler, speed, throughput
                with open('cdfFB5.csv', 'a') as f:
                    #write whole df[thruput] to the file
                    for i in df['Throughput']:
                        f.write(f"{scheduler},{i}\n")
                    print(f"Results have been saved to cdfFB5.csv")
                
                if seed == 1:
                    #open csv file and write the data to the file as scheduler, speed, throughput
                     with open('Ue0FB5seed1.csv', 'a') as f:
                        #write whole df[thruput] to the file based on IMSI value == 1   
                        for i, j in zip(df[df['IMSI'] == '1']['Throughput'], df[df['IMSI'] == '1']['end']):
                            #write end time for specific IMSI value 
                            f.write(f"{j},{scheduler},{i}\n")

        if buffermod == 0:
            #open csv file and write the data to the file as scheduler, speed, throughput
            with open('throughputWithoutFullBuffer.csv', 'a') as f:
                f.write(f"{scheduler},{speed},{throughput}\n")
                print(f"Results have been saved to throughputWithoutFullBuffer.csv")

            if speed == 0:
                #open csv file and write the data to the file as scheduler, speed, throughput
                with open('cdfNFB0.csv', 'a') as f:
                    #write whole df[thruput] to the file
                    for i in df['Throughput']:
                        f.write(f"{scheduler},{i}\n")
                    print(f"Results have been saved to cdfNFB0.csv")

                if seed == 1:
                    #open csv file and write the data to the file as scheduler, speed, throughput
                    with open('Ue0NFB0seed1.csv', 'a') as f:
                        #write whole df[thruput] to the file based on IMSI value == 1   
                        for i, j in zip(df[df['IMSI'] == '1']['Throughput'], df[df['IMSI'] == '1']['end']):
                            #write end time for specific IMSI value 
                            f.write(f"{j},{scheduler},{i}\n")
            
            if speed == 5:
                #open csv file and write the data to the file as scheduler, speed, throughput
                with open('cdfNFB5.csv', 'a') as f:
                    #write whole df[thruput] to the file
                    for i in df['Throughput']:
                        f.write(f"{scheduler},{i}\n")
                    print(f"Results have been saved to cdfNFB5.csv")

                if seed == 1:
                    #open csv file and write the data to the file as scheduler, speed, throughput
                    with open('Ue0NFB5seed1.csv', 'a') as f:
                        #write whole df[thruput] to the file based on IMSI value == 1 and store as end time from the file for specific IMSI value.
                        for i, j in zip(df[df['IMSI'] == '1']['Throughput'], df[df['IMSI'] == '1']['end']):
                            #write end time for specific IMSI value 
                            f.write(f"{j},{scheduler},{i}\n")



        # Specify the output file path
        output_file_path = log_file

        # Save the results to a new CSV file at different location
        df.to_csv(output_file_path, index=False)

        print(f"Results have been saved to {output_file_path}")