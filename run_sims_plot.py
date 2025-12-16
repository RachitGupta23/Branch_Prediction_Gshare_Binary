import os
import re
import subprocess
import argparse
import matplotlib.pyplot as plt

def run_simulation(model_type, m_value, trace_file, output_dir):
    """
    Runs the C++ executable './sim' with given parameters and saves output to a file.
    Returns the misprediction rate (float).
    """
    # Construct output filename
    base_trace = os.path.splitext(os.path.basename(trace_file))[0]
    output_file = os.path.join(output_dir, f"{model_type}_{base_trace}_m{m_value}.txt")

    # Create command
    cmd = ["./sim", model_type, str(m_value), trace_file]
    print(f"Running: {' '.join(cmd)} -> {output_file}")

    # Run the command and capture output
    with open(output_file, "w") as f:
        subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT)

    # Extract misprediction rate from the output file
    with open(output_file, "r") as f:
        content = f.read()

    match = re.search(r"misprediction\s*rate:\s*([\d.]+)\s*%", content, re.IGNORECASE)
    if match:
        return float(match.group(1))
    else:
        print(f"⚠️ Warning: No misprediction rate found in {output_file}")
        return None

def main():
    # Argument parser
    parser = argparse.ArgumentParser(description="Run sim with varying m and plot misprediction rate.")
    parser.add_argument("model_type", help="Model type argument for sim (e.g., bimodal, gshare, hybrid)")
    parser.add_argument("trace_file", help="Trace file name (e.g., gcc_trace.txt)")
    args = parser.parse_args()

    # Create directory for simulation outputs
    output_dir = "sim_runs"
    os.makedirs(output_dir, exist_ok=True)

    # Range of m values (7–20)
    m_values = list(range(7, 21))
    mispred_rates = []

    # Run all simulations and collect data
    for m in m_values:
        rate = run_simulation(args.model_type, m, args.trace_file, output_dir)
        mispred_rates.append(rate if rate is not None else float('nan'))

    # Plot the data
    plt.figure(figsize=(8, 5))
    plt.plot(m_values, mispred_rates, marker='o', linestyle='-', color='b')
    plt.title(f"{args.trace_file}, {args.model_type}")
    plt.xlabel("m")
    plt.ylabel("Misprediction Rate (%)")
    plt.grid(True)
    plt.tight_layout()

    # Save and show plot
    plot_filename = f"{args.model_type}_{os.path.splitext(args.trace_file)[0]}_plot.png"
    plt.savefig(plot_filename)
    print(f"\n📊 Plot saved as: {plot_filename}")
    plt.show()

if __name__ == "__main__":
    main()


