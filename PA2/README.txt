YUSUF BILGICH
11848141

-> How to compile and run your program
Use the following command to run the program:
gcc -Wall -o PA2-finalCode PA2-finalCode.c
./PA2-finalCode 64 16 input_sequential.txt
(replace the .txt file with your desired FILE)

-> Assumptions made, if any
Above given execution commands assumes the below configuration:
Cache size: 64 bytes
Block size: 16 bytes
Mapping: Direct-mapped
Number of cache lines: 64 / 16 = 4 lines
Address size: 32 bits
Each block holds addresses in ranges of 16 bytes (e.g., 0–15, 16–31, etc.)

-> Example input and output
Exmaple Input (input_sequential.txt):
0
4
8
12
16
20
24
28
32
36

Sample Output:
Total memory accesses: 10
Cache hits: 7
Cache misses: 3
Cache hit rate: 70.00%