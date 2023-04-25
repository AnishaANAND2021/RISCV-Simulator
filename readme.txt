This folder consists of 6 files for phase 1:
Instruction of running phase 3 is shown in phase 3 section

---------------------------------------Phase 1 ----------------------------------------
    1. input.txt -> It consists of the machine code in the formate: 0x0 0x00000093
                    The first number represents PC value and the second one is the 
                    instruction. It can be changes according to the requirement of 
                    the user. Whatever written on it will be executed in the code.
    
    2. Memory_file.txt -> It is showing our memory. The memory which is not written 
                          in the file is empty .It is also in the same formate that
                          of input file . The first value represents memory address.
                          And the second one is the value stored in that memeory. 
                          As the name suggests, the values are in hexadecimal.
    
    3. phase1.cpp -> It is the c++ code of the simulator for phase 1. After the code is executed,
                     number of cycles in each step ,total number of cycles and the 
                     description of each instruction are shown in the terminal.
    
    4. readme.txt -> Explanation
    
    5. register_file_dec -> Shows the values of 32 registers(in decimal) after the 
                            execution of the code.
    
    6. register_file_hex -> Shows the values of 32 registers(in hexadecimal) after 
                            the execution of the code.
    
    
---------------------------------------Phase 2 ----------------------------------------
    For phase 2, we have added some more files:
    
    1. phase2.cpp -> C++ code of the simulator (updated version)
    
    2. phase2_gui.py -> python code for phase2_gui
    
    3. terminal.txt -> showing the operations performed in each cycle
    
    4. output.txt -> giving basic information after running the code
    
    
---------------------------------------Phase 3 ----------------------------------------
    For phase 3, we have added other files :
    
    1. data_mem.txt -> shows data memory in the form of blocks
    
    2. d_$.txt -> shows a data cache in each cycle
    
    3. d_rl.txt -> shows recency list (after set no, it shows recency and then block no)
    
    4. inst_mem.txt -> shows instruction memory in the form of blocks
    
    5. i_$.txt -> shows a instruction cache in each cycle
    
    6. i_rl.txt -> shows recency list (after set no, it shows recency and then block no)
    
    7. phase3.cpp -> cpp code of phase 3
                     For running the code , open terminal and type the commands for c++ code.
                     It will ask $size and $ block size which must be given in kB.
                     Then it will ask for associativity (DM/FA/SA) , case senstive(only upper case)
                     In case of FA/SA ,it will ask for replacement policy(FIFO/LRU/LFU/RANDOM) ,case
                     sensitive
                     
    8. phase3_gui.py -> gui for phase 3
    
    -----------------------------------THANK YOU-----------------------------------
                                 Questions are Welcome !!
    