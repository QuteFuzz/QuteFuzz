import subprocess
import argparse
import os 
import sys
import shutil

QC_DIR = "quantum_circuits"
PLOTS_DIR = os.path.join(QC_DIR, "plots")

def progress_bar(current : int, total : int) -> None:
    # Calculate progress and lengths for the filled and empty parts of the bar
    progress = int((current / total) * 100)
    done = int(progress / 2)  
    left = 50 - done  

    # Build and print the progress bar
    bar = f"\rProgress : [{'▇' * done}{' ' * left}] Circuit {current}/{total}"
    sys.stdout.write(bar)
    sys.stdout.flush()

def setup_dir() -> None:
    if(os.path.exists(QC_DIR)):
        for file in os.listdir(QC_DIR):
            path = os.path.join(QC_DIR, file)
            if(os.path.isfile(path)):
                os.remove(path)
    else:
        os.mkdir(QC_DIR)
        print("Created", QC_DIR, "directory")

def main() -> int:
    parser = argparse.ArgumentParser(description="Runs QuteFuzz generator and differential tester")

    parser.add_argument("--f", type=str, help="Frontend to generate for ((q)iskit, (c)irq, (p)ytket)", default="p", choices=["qiskit","cirq","pytket","q","c","p"])
    parser.add_argument("--n", type=str, help="Number of programs to generate", default="1")
    parser.add_argument("-v", action="store_true",  help="Verbose adds extra information to the results log file")
    parser.add_argument("-p", action="store_true", help="Plot graphs")

    args = parser.parse_args()

    setup_dir()

    # vars passed to python circuits for verbose results and plotting
    verbose = "-v" if (args.v) else ""
    plot = "-p" if (args.p) else ""

    # create plots directory if plotting is enabled
    if args.p:
        # Clear the plots directory if it exists
        if os.path.exists(PLOTS_DIR):
            shutil.rmtree(PLOTS_DIR)
        # Create the plots directory
        os.mkdir(PLOTS_DIR)
    # Clear the plots directory if plotting is disabled
    elif(os.path.exists(PLOTS_DIR) and not args.p):
        shutil.rmtree(PLOTS_DIR)

    # compile the generate
    subprocess.run("make")

    if(os.name == "nt"):
        exe = "gen.exe"
    elif (os.name == "posix"):
        exe = "./gen"

    if not os.path.exists(exe):
        print("Issue while compiling, could not find executable", exe)
        return -1

    subprocess.run([exe, "-n", args.n, f"-{args.f[0]}"])

    # run circuits
    print("Running cirucits ....")
    for i, file in enumerate(sorted(os.listdir(QC_DIR), key = lambda x: int(x.split('.')[0][7:]) if "py" in x else 0)):
        path = os.path.join(QC_DIR, file)
        
        if (not os.path.isdir(path) and (file.split(".")[1] == "py")):
            
            log_path = os.path.join(QC_DIR, "_results.txt")

            progress_bar(i+1, int(args.n))

            with open(log_path, "a") as f:
                
                try:
                    subprocess.run(
                        ["python3", "-Wi", path, verbose, plot],
                        stdout=f,
                        stderr=subprocess.STDOUT,
                        check=True
                    )
                except Exception as e:
                    print(f"\nERROR '{e}' occurred while running circuits, check", log_path, "for details")

    print("\nResults in ", log_path)
    return 0

if __name__ == "__main__":

    main()  