import pandas as pd
import matplotlib.pyplot as plt
import os
from matplotlib.ticker import LogLocator
import string

def extract_message_size(lines):
    for line in lines:
        if 'size of one line in bytes:' in line:
            return int(line.split(':')[-1].strip().replace('b', ''))
        elif 'size of one line in kilobytes:' in line:
            return float(line.split(':')[-1].strip().replace('kb', '')) * 1024
    return None  # Default fallback if no size is found

def read_and_process_file(filepath):
    with open(filepath, 'r') as file:
        lines = file.readlines()

    message_size = extract_message_size(lines)
    data = []
    for line in lines:
        if 'Cycles/' in line and ('insert()' in line or 'find()' in line or 'erase()' in line or '++it' in line):
            action = line.split('/')[2].split()[0]  # Get the action from the Metric name
            metric_name = line.strip().split(',')[0]
            parts = line.split(',')
            min_val = float(parts[1].strip())
            avg_val = float(parts[2].strip())
            max_val = float(parts[3].strip())
            pct_90 = float(parts[4].strip())
            pct_50 = float(parts[5].strip())
            pct_10 = float(parts[6].strip())
            data.append((action, metric_name, message_size, min_val, avg_val, max_val, pct_90, pct_50, pct_10))
    return data

def sanitize_filename(text):
    """Sanitize the filename to remove unwanted characters."""
    valid_chars = "-_() %s%s" % (string.ascii_letters, string.digits)
    cleaned_filename = ''.join(c for c in text if c in valid_chars)
    return cleaned_filename

def plot_data(data, title):
    """Generate plots for the given data and save them with sanitized filenames."""
    df = pd.DataFrame(data, columns=['Action', 'Metric', 'Message Size', 'Min', 'Average', 'Max', '90th Percentile',
                                     '50th Percentile', '10th Percentile'])
    for action, action_df in df.groupby('Action'):
        action_df.set_index('Message Size', inplace=True)
        sorted_df = action_df.sort_index()  # Sort the DataFrame by the index (Message Size)

        ax = sorted_df[['Min', 'Average', 'Max', '90th Percentile', '50th Percentile', '10th Percentile']].plot(
            kind='line', figsize=(14, 8), marker='o')
        ax.set_yscale('log')

        # Setting minor ticks using LogLocator
        minor_locator = LogLocator(base=10.0, subs='auto', numticks=10)
        ax.yaxis.set_minor_locator(minor_locator)
        ax.yaxis.set_minor_formatter(plt.FuncFormatter(lambda x, _: f'{x:.1f}'))

        plt.title(f"{title} - {action}")
        plt.ylabel('Cycles (log scale)')
        plt.xlabel('Message Size (bytes)')
        plt.grid(True, which="both", ls="--")
        plt.legend(title='Percentiles and Values')
        plt.tight_layout()

        # Sanitize and format the filename
        filename = f"{sanitize_filename(title).replace(' ', '_').lower()}_{sanitize_filename(action).lower()}.png"
        plt.savefig(filename)
        plt.close()


# Directory containing the benchmark files
directory = "../benchmarks"

# Loop through each file in the directory
all_data = []
for filename in os.listdir(directory):
    if filename.endswith('.txt'):
        filepath = os.path.join(directory, filename)
        file_data = read_and_process_file(filepath)
        all_data.extend(file_data)

# Assuming data structure names are part of the metric names, e.g., map, tree, etc.
structures = set(metric_name.split('/')[0] for _, metric_name, *_ in all_data)

for structure in structures:
    structure_data = [item for item in all_data if item[1].split('/')[0] == structure]
    plot_data(structure_data, f"{structure.capitalize()} Cycles Metrics")