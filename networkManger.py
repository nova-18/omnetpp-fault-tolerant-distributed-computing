import re
import ast
import os
def edit_ini_file(file_path, num_clients, num_servers):
  
    try:
        # Read the input file
        with open(file_path, 'r') as file:
            lines = file.readlines()
        
        # Create a copy of lines to modify
        output_lines = []
        
        # Flag to track if we're in the [General] section
        in_general_section = False
        
        # Process each line
        for line in lines:
            # Check for section headers
            if line.strip().startswith('['):
                in_general_section = line.strip() == '[General]'
            
            # If in General section, modify specific parameters
            if in_general_section:
                if '**.numClients' in line:
                    output_lines.append(f"**.numClients = {num_clients}\n")
                elif '**.numServers' in line:
                    output_lines.append(f"**.numServers = {num_servers}\n")
                else:
                    output_lines.append(line)
            else:
                # Keep lines outside General section unchanged
                output_lines.append(line)
        
        # Write the modified content back to the same file
        with open(file_path, 'w') as file:
            file.writelines(output_lines)
        
        print(f"Successfully edited parameters in {file_path}")
    
    except IOError as e:
        print(f"Error reading or writing file: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

def edit_ned_connections(file_path, new_connections):
    """
    Edit the connections section of a .ned file by replacing existing connections 
    with new provided connections, modifying the file in-place.
    
    :param file_path: Path to the .ned file to be modified
    :param new_connections: List of new connection strings to replace existing connections
    """
    try:
        # Read the input file
        with open(file_path, 'r') as file:
            lines = file.readlines()
        
        # Find the start of the connections section
        connections_start = -1
        for i, line in enumerate(lines):
            if 'connections :' in line.lower():
                connections_start = i
                break
        
        if connections_start == -1:
            raise ValueError("No 'connections:' section found in the file")
        
        # Find the end of the connections section (last line before closing brace)
        connections_end = len(lines) - 1
        for i in range(connections_start + 1, len(lines)):
            if lines[i].strip() == '}':
                connections_end = i
                break
        
        # Prepare new connections with two tabs (8 spaces) of indentation
        new_connection_lines = ['\t\t' + conn + ';\n' for conn in new_connections]
        
        # Reconstruct the file
        output_lines = (
            lines[:connections_start + 1] +  # Keep the 'connections:' line
            new_connection_lines +           # Add new connections with proper indentation
            lines[connections_end:]          # Keep the closing brace and any following lines
        )
        
        # Write the modified content back to the same file
        with open(file_path, 'w') as file:
            file.writelines(output_lines)
        
        print(f"Successfully edited connections in {file_path}")
    
    except IOError as e:
        print(f"Error reading or writing file: {e}")
    except ValueError as e:
        print(f"Error processing file: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

def parse_topology_file(filename):
    data = {}
    current_section = None
    
    data_List = []
    with open(filename, 'r') as file:
        for line in file:
            line = line.strip()
            if not line or line.startswith("#") or line == "":  # Ignore empty lines and comments
                continue
            data_List.append(line)

        for index in range(len(data_List)):
            val = data_List[index]
            
            if val == '{' :
                key = data_List[index-1]
                data[key] = []

                for j in range(index+1,len(data_List)):
                    ele = data_List[j]
                    if(ele == '}'): 
                        break
                    data[key].append(ele)
            
        for key in data.keys():
            if(key[0] == '$'):
                lst = data[key]
                lst = [ast.literal_eval(item) if isinstance(item, str) else item for item in lst]
                data[key] = lst        

        data['clients'] = int(data['parameters'][0].split(' ')[-1])
        data['servers'] = int(data['parameters'][1].split(' ')[-1])
    return data

# Example usage
file_path = "topology.txt"
topology = parse_topology_file(file_path)
print(topology)


ned_file = "network.ned"
ini_file = "omnetpp.ini"
connections_list = []

# Define log directory and file names
log_dir = "logs"
log_files = ["client.log", "server.log", "gossip.log"]

log_dir = "logs"
log_files = ["client.log", "server.log", "gossip.log"]

# Create logs directory if it doesn't exist
if not os.path.exists(log_dir):
    os.makedirs(log_dir)
    print(f"Created directory: {log_dir}")

# Create or wipe log files
for log_file in log_files:
    log_path = os.path.join(log_dir, log_file)
    with open(log_path, "w") as f:  # Open in write mode to wipe existing content
        f.write("")  
    print(f"Wiped/Created file: {log_path}")
# Ask the user for Total_Task_Count using a prompt
try:
    Total_Task_Count = int(input("Enter the Total Task Count (integer value): "))
    
    # Generate config.h
    with open('config.h', 'w') as f:
        f.write(f'#ifndef CONFIG_H\n#define CONFIG_H\n\nconst int Total_Task_Count = {Total_Task_Count};\n\n#endif')
    
    print(f"config.h generated with Total_Task_Count = {Total_Task_Count}")
except ValueError:
    print("Invalid input! Please enter an integer.")


for i in range(topology['clients']):
    for j in range(topology['clients']):
        cmd = f"clients[{i}].clientOut++ --> {{ delay = {topology['$client -> client delay'][i][j]}ms; }} --> clients[{j}].clientIn++ "
        connections_list.append(cmd)

for i in range(topology['clients']):
    for j in range(topology['servers']):

        cmd = f"clients[{i}].serverOut++ --> {{ delay = {topology['$client -> server delay'][i][j]}ms; }} --> servers[{j}].in++ "
        connections_list.append(cmd)

    
for j in range(topology['servers']):
    for i in range(topology['clients']):

        cmd = f"clients[{i}].serverIn++ <-- {{ delay = {topology['$server -> client delay'][i][j]}ms; }} <-- servers[{j}].out++ "
        connections_list.append(cmd)

edit_ini_file(ini_file, topology['clients'], topology['servers'])
edit_ned_connections(ned_file, connections_list)

