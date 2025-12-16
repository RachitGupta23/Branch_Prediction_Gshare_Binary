import os
import subprocess

def main():
    # Current working directory
    cwd = os.getcwd()
    trace_dir = os.path.join(cwd, "val_traces")
    results_dir = os.path.join(cwd, "results")
    diff_file = os.path.join(cwd, "diff_results.txt")

    # Create results directory if it doesn't exist
    os.makedirs(results_dir, exist_ok=True)

    # Clear old diff file
    if os.path.exists(diff_file):
        os.remove(diff_file)

    # Iterate over all files in val_traces directory
    for filename in os.listdir(trace_dir):
        if filename.startswith("val") and filename.endswith(".txt"):
            # Example: val_bimodal_1.txt, val_gshare_2.txt, val_hybrid_3.txt
            parts = filename.split(".")
            if len(parts) < 2:
                continue

            # Extract test number
            file_number = parts[0].split("_")[-1]

            # Extract algorithm name
            algo_name = parts[0].split("_")[1] if "_" in parts[0] else "unknown"

            trace_path = os.path.join(trace_dir, filename)

            # Read the validation file to get the command line
            with open(trace_path, "r") as f:
                lines = f.readlines()

            # Find the command line (starts with './sim')
            cmd_line = None
            for line in lines:
                if line.strip().startswith("./sim"):
                    cmd_line = line.strip()
                    break

            if not cmd_line:
                print(f"⚠️ No valid command found in {filename}, skipping.")
                continue

            # Split command into arguments
            cmd = cmd_line.split()
            output_file = os.path.join(results_dir, f"print_result_{algo_name}_{file_number}.txt")

            print(f"Running: {' '.join(cmd)} -> {output_file}")

            # Run the simulator and capture output
            with open(output_file, "w") as out:
                subprocess.run(cmd, stdout=out, stderr=subprocess.STDOUT)

            # Run diff -iw (ignore case and whitespace)
            diff_cmd = ["diff", "-iw", output_file, trace_path]
            diff_process = subprocess.run(
                diff_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True
            )

            # Write any differences to diff_results.txt
            if diff_process.stdout.strip():  # Non-empty diff
                with open(diff_file, "a") as df:
                    df.write(f"✗ Difference found for {filename}:\n")
                    df.write(diff_process.stdout)
                    df.write("\n")
            else:
                with open(diff_file, "a") as df:
                    df.write(f"✓ {filename} passed validation.\n")

if __name__ == "__main__":
    main()

