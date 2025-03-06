# OpenFHE CKKS Benchmarks

This project provides a straightforward set of benchmarks for evaluating the performance of fundamental CKKS (Cheon-Kim-Kim-Song) homomorphic encryption operations using the OpenFHE library.

**Important Note:** These benchmarks were specifically tested and validated with **OpenFHE v1.2.3**. Results may vary with other versions of the library. Moreover, if you want to use another OpenFHE version, you need to specify the specific version number in the `CMakeLists.txt` file in the `find_package` cmake command.


## Purpose

The primary goal of these benchmarks is to measure and analyze the execution time of core CKKS operations, including:

* **Homomorphic Addition and Multiplication:** Assess the efficiency of basic arithmetic operations on encrypted data.
* **Bootstrapping:** Evaluate the performance of the bootstrapping procedure, a critical component for performing complex computations on encrypted data.
* **Unencrypted Vector Operations:** Provide a baseline for comparison by benchmarking equivalent vector operations in plaintext.

## Prerequisites

Before running the benchmarks, ensure that you have the OpenFHE library properly installed. The project expects the OpenFHE installation to reside in a directory named `myinstalledlib`, located directly outside the project directory.

**Directory Structure:**  
Your Working Directory
```bash  
|__ myinstalledlib # this should contain the installed OpenFHE v1.2.3 library  
    |__ include  
    |__ lib  
|__ OpenFHE-Benchmarks  
```

**OpenFHE Installation:**

* Verify that OpenFHE is correctly built and installed within the `myinstalledlib` directory.
* Ensure that the necessary OpenFHE libraries and headers are accessible.

## Building the Benchmarks

To build the benchmarks, follow these steps:

1.  **Create a build directory:**

    ```bash
    mkdir build
    cd build
    ```

2.  **Run CMake:**

    ```bash
    cmake ..
    ```

    This command configures the build process using CMake, searching for the OpenFHE library in the specified location.

3.  **Compile the benchmarks:**

    ```bash
    make
    ```

    This command compiles the source code and generates the executable benchmark programs.

## Executable Targets

The build process will produce the following executable targets within the `build` directory:

* **`bench-add-mul`:** This benchmark evaluates the performance of basic CKKS homomorphic addition and multiplication operations. It provides insights into the efficiency of performing arithmetic on encrypted data.
* **`bench-boots`:** This benchmark measures the execution time of the CKKS bootstrapping procedure. Bootstrapping is essential for maintaining the noise of encrypted computations and enabling complex operations.
* **`bench-add-mul-unencrypted`:** This benchmark performs equivalent vector addition and multiplication operations on unencrypted data. It serves as a baseline for comparing the performance of homomorphic operations with their plaintext counterparts. Note no bootstrapping is required in unencerypted computation.

## Running the Benchmarks

To run a specific benchmark, simply execute the corresponding executable from the `build` directory:

```bash
./bench-add-mul
./bench-boots
./bench-add-mul-unencrypted
```

## Sample Output - Single-Thread Build


**`bench-add-mul`:**

============ Performance Profiling Results ============  
| Operation                          | First Run (ms) | Avg (excl. first) (ms) | Memory Overhead (ms) |
| ------------------------------------ | --------------- | ------------------------- | --------------------- |
| MakeCKKSPackedPlaintext           | 17.832          | 17.850                    | -0.019                |
| Encrypt                             | 61.898          | 73.336                    | -11.438               |
| EvalAdd                             | 8.894           | 10.316                    | -1.421                |
| EvalSub                             | 19.142          | 18.395                    | 0.748                 |
| EvalMult (scalar)                  | 55.711          | 34.744                    | 20.967                |
| EvalMult (ciphertext)             | 210.050         | 243.218                   | -33.168               |
| EvalMultNoRelin                    | 129.145         | 95.695                    | 33.450                |
| Relinearize                       | 210.327         | 188.345                   | 21.982                |
| EvalRotate (1)                    | 226.849         | 201.260                   | 25.589                |
| EvalRotate (-2)                   | 153.061         | 195.642                   | -42.581               |
| Decrypt                            | 167.451         | 143.760                   | 23.691                |

<br>

**`bench-boots`:**

Bootstrapping is slow, no need for the first run time. Ignore it.

============ Performance Profiling Results ============
| Operation           | First Run (ms) | Avg (excl. first) (ms) | Memory Overhead (ms) |
| -------------------- | --------------- | ------------------------- | --------------------- |
| EvalBootstrap       | 0.000          | 45426.663                | -45426.663           |

<br>

**`bench-add-mul-unencrypted`:**

============ Performance Profiling Results ============  
| Operation         | First Run (ms) | Avg (excl. first) (ms) | Memory Overhead (ms) |
| ----------------- | --------------- | ------------------------- | --------------------- |
| Add UnEnc         | 0.103          | 0.013                      | 0.090                |
| Sub UnEnc         | 0.057          | 0.011                      | 0.045                |
| Mult (scalar)     | 0.050          | 0.009                      | 0.040                |
| Mult UnEnc        | 0.058          | 0.012                      | 0.046                |

<br>