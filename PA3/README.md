YUSUF BILGICH
11848141

-> How to compile and run your program
Use the following command to run the program:
gcc -Wall -o PA3-finalCode PA3-finalCode.c
./PA3-finalCode processes.txt 2
(replace the .txt file with your desired input FILE,
also the 2 in this case is is the assumed Round Robin time quantum)

-> Assumptions made, if any
FCFS is non-preemptive
SJF is non-preemptive
RR is preemptive
In all cases, the tiebraker is pid if arrival time matches
Also, make sure your text file is formatted correctly 
(No headers for PID, Arrival, and Burst; only the data directly)

-> Example input and output
Exmaple Input (processes.txt):
1 0 5
2 1 3
3 2 8
4 3 6

Sample Output:
===== First-Come, First-Serve =====
P1 [0 -> 5]
P2 [5 -> 8]
P3 [8 -> 16]
P4 [16 -> 22]

PID     Arrival Burst   Waiting Turnaround
1       0       5       0       5
2       1       3       4       7
3       2       8       6       14
4       3       6       13      19

Average Waiting Time: 5.75
Average Turnaround Time: 11.25

===== Shortest Job First =====
P1 [0 -> 5]
P2 [5 -> 8]
P4 [8 -> 14]
P3 [14 -> 22]

PID     Arrival Burst   Waiting Turnaround
1       0       5       0       5
2       1       3       4       7
3       2       8       12      20
4       3       6       5       11

Average Waiting Time: 5.25
Average Turnaround Time: 10.75

===== Round Robin (q = 2) =====
P1 [0 -> 2]
P2 [2 -> 4]
P3 [4 -> 6]
P1 [6 -> 8]
P4 [8 -> 10]
P2 [10 -> 11]
P3 [11 -> 13]
P1 [13 -> 14]
P4 [14 -> 16]
P3 [16 -> 18]
P4 [18 -> 20]
P3 [20 -> 22]

PID     Arrival Burst   Waiting Turnaround
1       0       5       9       14
2       1       3       7       10
3       2       8       12      20
4       3       6       11      17

Average Waiting Time: 9.75
Average Turnaround Time: 15.25