#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100

/* -----------------------------
   Process structure
-------------------------------- */
typedef struct {
    int pid;
    int arrival_time;
    int burst_time;

    /* Calculated fields */
    int remaining_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
} Process;

/* -----------------------------
   Function Prototypes
-------------------------------- */
int read_processes(const char* filename, Process processes[]);
void reset_processes(Process processes[], int n);

void fcfs(Process processes[], int n);
void sjf(Process processes[], int n);
void round_robin(Process processes[], int n, int quantum);

void print_metrics(Process processes[], int n);
void print_gantt(int pid, int start, int end);

/* -----------------------------
   Utility: sort by arrival time
-------------------------------- */
// ~TODO~ (DONE):
// Use this function in junction with qsort to sort processes by arrival time
int compare_arrival(const void *a, const void *b) {
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;
    
    // Sorting by arrival time
    if (p1->arrival_time != p2->arrival_time) {
        return p1->arrival_time - p2->arrival_time;
    }
    // If arrived at the identical ms, sort by PID as secondary precaution
    return p1->pid - p2->pid;
}

/* -----------------------------
   Main
-------------------------------- */
int main(int argc, char* argv[]) {
    Process processes[MAX_PROCESSES];
    int n;

    if (argc < 3) {
        printf("Usage: %s <input_file> <time_quantum>\n", argv[0]);
        return 1;
    }

    int quantum = atoi(argv[2]);

    n = read_processes(argv[1], processes);
    if (n <= 0) {
        printf("No processes loaded.\n");
        return 1;
    }

    // We can use qsort here with the help of our compare_arrival function
    // to properly sort the processes by their arrival time before starting scheduling
    qsort(processes, n, sizeof(Process), compare_arrival);

    printf("\n===== First-Come, First-Serve =====\n");
    reset_processes(processes, n);
    fcfs(processes, n);
    print_metrics(processes, n);

    printf("\n===== Shortest Job First =====\n");
    reset_processes(processes, n);
    sjf(processes, n);
    print_metrics(processes, n);

    printf("\n===== Round Robin (q = %d) =====\n", quantum);
    reset_processes(processes, n);
    round_robin(processes, n, quantum);
    print_metrics(processes, n);

    return 0;
}

/* -----------------------------
   Read input file
-------------------------------- */
int read_processes(const char* filename, Process processes[]) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening file");
        return -1;
    }

    int count = 0;
    while (fscanf(fp, "%d %d %d",
        &processes[count].pid,
        &processes[count].arrival_time,
        &processes[count].burst_time) == 3) {

        processes[count].remaining_time = processes[count].burst_time;
        count++;
    }

    fclose(fp);
    return count;
}

/* -----------------------------
   Reset calculated fields
-------------------------------- */
void reset_processes(Process processes[], int n) {
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].completion_time = 0;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
    }
}

/* -----------------------------
   FCFS Scheduling
-------------------------------- */
void fcfs(Process processes[], int n) {
    
	
	int current_time = 0;

    for (int i = 0; i < n; i++) {
        if (current_time < processes[i].arrival_time) {
            current_time = processes[i].arrival_time;
        }

        int start = current_time;
        current_time += processes[i].burst_time;
        int end = current_time;

        print_gantt(processes[i].pid, start, end);
        processes[i].completion_time = end;
    }
}

/* -----------------------------
   SJF Scheduling (Non-preemptive)
-------------------------------- */
void sjf(Process processes[], int n) {
    // ~TODO~ (DONE):
    int current_time = 0;
    int completed = 0;

    // 1. While not all processes are completed:
    while (completed < n) {
        int shortest_index = -1;
        // 2. Find the available process with the shortest burst time
        for (int i = 0; i < n; i++) {
            // Check if process has arrived AND is not yet finished
            if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0) {
                // If it's the first one we found, or if it has a shorter burst time (as per SJF)
                if (shortest_index == -1 || processes[i].burst_time < processes[shortest_index].burst_time) {
                    shortest_index = i;
                }
            }
        }
        // 3. Execute it fully...
        if (shortest_index != -1) {
            int start = current_time;
            current_time += processes[shortest_index].burst_time;
            int end = current_time;

            print_gantt(processes[shortest_index].pid, start, end);

            // ...and update completion_time
            processes[shortest_index].completion_time = end;
            processes[shortest_index].remaining_time = 0; // Marked done (no more time left)
            completed++; // increment completed (one step closer to finishing all processes)
        } 
        // 4. Handle CPU idle time if no process is available
        else {
            current_time++; // No suitable process, advancing the clock
        }
    }
}

/* -----------------------------
   Round Robin Scheduling
-------------------------------- */
void round_robin(Process processes[], int n, int quantum) {
    int current_time = 0;
    int completed = 0;

    // ~TODO~ (DONE):
    // 1. Implement a ready queue (circular array)
    int queue[MAX_PROCESSES + 1]; 
    int front = 0, rear = 0;

    // This queue will track the added processes (to avoid multiples)
    int added[MAX_PROCESSES];
    for (int i = 0; i < n; i++) {
        added[i] = 0;
    }

    while (completed < n) {
        // 2. Add processes to queue when arrival_time <= current_time
        for (int i = 0; i < n; i++) {
            if (!added[i] && processes[i].arrival_time <= current_time) {
                queue[rear] = i;
                // Use module to wrap around (remember, circular queue)
                rear = (rear + 1) % (MAX_PROCESSES + 1);
                added[i] = 1;
            }
        }

        // CPU is idle if no change, advance time
        if (front == rear) {
            current_time++;
            continue;
        }
        // Otherwise we can grab the next process
        int idx = queue[front];
        front = (front + 1) % (MAX_PROCESSES + 1);

        // Now that we have handled the prior requirements and grabbed the next process, we can:
        // 3. Execute each process for min(quantum, remaining_time)
        int exec = (processes[idx].remaining_time > quantum) 
            ? quantum 
            : processes[idx].remaining_time;

        int start = current_time;
        current_time += exec;
        int end = current_time;

        print_gantt(processes[idx].pid, start, end);
        // Deduct the executed time from remaining_time
        processes[idx].remaining_time -= exec;

        // If a process has arrived during the exection,
        // they get priority to be added to the queue (before requeuing, if needed)
        for (int i = 0; i < n; i++) {
            if (!added[i] && processes[i].arrival_time <= current_time) {
                queue[rear] = i;
                rear = (rear + 1) % (MAX_PROCESSES + 1);
                added[i] = 1;
            }
        }

        // 4. Re-queue unfinished processes...
        if (processes[idx].remaining_time > 0) {
            queue[rear] = idx;
            rear = (rear + 1) % (MAX_PROCESSES + 1);
        } // ...or 5. Track completion_time correctly
        else {
            processes[idx].completion_time = current_time;
            completed++;
        }
    }
}

/* -----------------------------
   Print Metrics
-------------------------------- */
void print_metrics(Process processes[], int n) {
    double total_wait = 0;
    double total_turnaround = 0;

    printf("\nPID\tArrival\tBurst\tWaiting\tTurnaround\n");

    for (int i = 0; i < n; i++) {
        processes[i].turnaround_time =
            processes[i].completion_time - processes[i].arrival_time;

        processes[i].waiting_time =
            processes[i].turnaround_time - processes[i].burst_time;

        total_wait += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;

        printf("%d\t%d\t%d\t%d\t%d\n",
            processes[i].pid,
            processes[i].arrival_time,
            processes[i].burst_time,
            processes[i].waiting_time,
            processes[i].turnaround_time);
    }

    printf("\nAverage Waiting Time: %.2f\n", total_wait / n);
    printf("Average Turnaround Time: %.2f\n", total_turnaround / n);
}

/* -----------------------------
   Gantt Chart Helper
-------------------------------- */
void print_gantt(int pid, int start, int end) {
    printf("P%d [%d -> %d]\n", pid, start, end);
}