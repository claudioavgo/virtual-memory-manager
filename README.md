# Virtual Memory Manager

## Overview
This project is a virtual memory manager implementation designed as part of an Operating Systems course. The primary goal of this project is to simulate a virtual memory manager that can handle page replacement algorithms and translate virtual addresses to physical addresses. This project is based on the concepts outlined in "Operating System Concepts" by Silberschatz.

## Features
- **Page Replacement Algorithms**: Implements FIFO (First-In-First-Out) and LRU (Least Recently Used) algorithms for page replacement.
- **Translation Lookaside Buffer (TLB)**: Simulates a TLB with FIFO replacement to optimize address translation.
- **Virtual to Physical Address Translation**: Converts virtual addresses to physical addresses using page tables.
- **Memory Management**: Simulates physical memory and handles page faults.

## Files Included
- **main.c**: The main source code file that contains the implementation of the virtual memory manager.
- **BACKING_STORE.bin**: A binary file representing the secondary storage with 256 pages, each containing 256 bytes.
- **Makefile**: Contains build instructions to compile the project.
- **README.md**: This documentation file.
- **correct.txt**: Output file containing the results of the virtual address translations.

## How to Run
1. **Compile the project**:
   ```sh
   make
   ```

2. **Execute the program**:
   ```sh
   ./vm_manager <input_file>
   ```
   - Replace `<input_file>` with the path to the file containing virtual addresses to be translated.

3. **Output**:
   - The program will generate a file named `correct.txt` containing the virtual address translations, page faults, and TLB hits.

## Usage Example
Suppose you have an input file named `addresses.txt` containing virtual addresses:

```sh
./vm_manager addresses.txt
```

The output will be saved in `correct.txt`, showing each virtual address, its corresponding physical address, and details about page faults and TLB hits.

## Implementation Details
- **Page Structure**: 
  - Each page contains a virtual address, offset, page number, value, and hit count.
  
- **Frame Structure**: 
  - Each frame holds a page and tracks its last access time for LRU, as well as a hit count.
  
- **TLB**: 
  - A simple FIFO TLB is implemented to cache frequently accessed pages and reduce page table lookups.

## Configuration
- **Page Size**: 256 bytes
- **Number of Pages**: 256
- **Physical Memory Size**: 65,536 bytes
- **TLB Size**: Configured dynamically but currently using a FIFO queue with up to 16 entries.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Author
- **Cláudio Alves Gonçalves de Oliveira**  
  - Email: hi@claudioav.com  
  - Version: 1.0  
  - Date: 21/05/2024

## Acknowledgments
This project was inspired by exercises from the book "Operating System Concepts" by Silberschatz. Special thanks to my instructor and peers for their guidance and feedback.
