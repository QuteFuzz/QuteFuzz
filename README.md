# QuteFuzz

<p align="center" width="100%">
    <img width="25%" src="images/qutefuzz.png">

QuteFuzz generates circuits for Pytket, Cirq, and Qiskit directly. 

## Running the fuzzer

**Install python dependencies**
```sh
python3 -m pip install -r requirements.txt
```
This installs all the helper functions we use for differential testing as well as all other dependencies required to run the circuits such as qiskit, pytket and cirq libraries.

**Generate and run circuits**
```sh
python3 run.py
```

This generates and runs a Pytket circuit by default

- To see all available flags, use `python3 run.py -h`.
- You can run the generator on its own by compiling using `make`. Generate n circuits using:
    ```sh
    ./gen -n {n}
    ```
- Full usage: `./gen -n {programs to generate} [-q/-p/-c]`
- If you want to monitor the generation steps more closely, run `make clean` then `make dev` which will allow the program to print information during circuit generation. `-g` flag is also added so a debugger can be used as well.

All quantum circuits, logs and results will be shown in a `quantum_circuits` folder in the repo directory

## Bugs found with the help of QuteFuzz

### Pytket:

| Compiler Bugs | Simulator Bugs | Not implemented |
|---------------|----------------|--------|
| [Issue 1553 &#x1F41E;](https://github.com/CQCL/tket/issues/1553)  | [Issue 375 &#x1F41E;](https://github.com/CQCL/pytket-qiskit/issues/375) | [Issue 374 &#x1F41E;](https://github.com/CQCL/pytket-qiskit/issues/374) |
| [Issue 1566  &#x1F41E;](https://github.com/CQCL/tket/issues/1566) | 
| [Issue 1534  &#x1F41E;](https://github.com/CQCL/tket/issues/1534) (Known) |
| [Issue 1536  &#x1F41E;](https://github.com/CQCL/tket/issues/1536) |
| [Issue 1552  &#x1F41E;](https://github.com/CQCL/tket/issues/1552) | | 
| [Issue 1554  &#x1F41E;](https://github.com/CQCL/tket/issues/1554) | |
| | | |

### Qiskit:
| Compiler Bugs | Simulator Bugs |
|---------------|----------------|
| [Issue 13118 &#x1F41E;](https://github.com/Qiskit/qiskit/issues/13118) | [Issue 2230 &#x1F41E;](https://github.com/Qiskit/qiskit-aer/issues/2230) | 
| [Issue 13079 &#x1F41E;](https://github.com/Qiskit/qiskit/issues/13079) | [Issue 2215 &#x1F41E;](https://github.com/Qiskit/qiskit-aer/issues/2215)(Unacknowledged) |
| [Issue 13162 &#x1F41E;](https://github.com/Qiskit/qiskit/issues/13162) | |
| [Issue 13165 &#x1F41E;](https://github.com/Qiskit/qiskit/issues/13165)(Unacknowledged) |  |

### Cirq:

| Compiler Bugs | Simulator Bugs | Circuit Construction |
|---------------|----------------|--------|
| [Issue 6507 &#x1F41E;](https://github.com/quantumlib/Cirq/issues/6507) (Known) | [Issue 6730 &#x1F41E;](https://github.com/quantumlib/Cirq/issues/6730) | [Issue 6727 &#x1F41E;](https://github.com/quantumlib/Cirq/issues/6727) |

## Contributors

| People | GitHub | Email | 
|--------|--------|-------|
| Benny  | [:octocat:](https://github.com/Bennybenassius) | benny.liu21@imperial.ac.uk |
| Ilan  | [:octocat:](https://github.com/IlanIwumbwe) | ii122@imperial.ac.uk |
| Dr. John Wickerson  | [:octocat:](https://github.com/johnwickerson) | j.wickerson@imperial.ac.uk |

## A note on constants chosen

See [`headers/utils.h`](headers/utils.h)

The constants were chosen with a preference towards generation of smaller individual circuits. We reasoned that if we can find bugs in smaller circuits, then this is still great. We often had to reduce the test cases before filing bug reports so this process was made easier too. Moreover, smaller circuits run much faster, and are quicker to generate, which reduces the time of our testing loop. We tried to focus on making the generator capable of generating expressive circuits such that if we generated a large volume of circuits, there would be some that could potentially cause problems for the compiler.   

Nonetheless, we believe there is still some exploration that can be done. 