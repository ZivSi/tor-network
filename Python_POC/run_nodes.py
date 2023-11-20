import multiprocessing
import subprocess
import time


def run_node():
    try:
        subprocess.run(['python', 'node.py'], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running node.py: {e}")


def run_client():
    try:
        subprocess.run(['python', 'Client.py'], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running Client.py: {e}")


if __name__ == "__main__":
    num_processes = 15  # Change this to the desired number of parallel runs

    # Create a list of processes
    processes = [multiprocessing.Process(target=run_node) for _ in range(num_processes)]
    processes.append(multiprocessing.Process(target=run_client))

    # Start each process
    for i, process in enumerate(processes):
        if i == len(processes) - 1:
            time.sleep(3)

        process.start()

    # Wait for all processes to complete
    for process in processes:
        process.join()
