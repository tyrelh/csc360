## Tyrel Hiebert
CSC 360 - Assignment 3 - July 15, 2018\
Roundrobin CPU Scheduling
## Task 2 - Data Analysis
### Simulation, Data Aggrigation, & Statistics
I used a python script to run my simulations. First, it will build a string for each combination of quantum and dispatch times of the c commands *simgen* and *rrsim*, using 2000 processes and 8825 for the seed of *simgen*, and piping the results of *simgen* into *rrsim*. After each string is built, it will use Python3's *subprocess* module to run the processes and return the output. All the outputs are accumulated in order\
Next it will search the output for the *EXIT* events for all the tasks of a given trial, pull out their wait time and turnaround time, and add these to lists.
Then it will  calculate the averages of the wait time list and the turnaround time list.
Finally, the script will format the output to have each trial on a line with its quantum, dispatch, wait, and turnaround time. This formatted output is then saved to a text file to be read by the graphing script.

### Graphing & Analysis
The data is then loaded into JavaScript. I separate the data into a quantum list, dispatch list, wait time list, and dispatch time list. Then, using a library called [Chart.js](https://www.chartjs.org/), I plot the data using a line graph. I set up the two graphs (for wait and turnaround time) so that the results for each quantum were plotted separately using different colours. I then printed each graph separately to pdf using Chrome to produce the deliverables required.

[A live example can be found on my website](https://tyrelh.github.io/csc360/a3/).

From the graphs I can see that there is a very linear relationship between wait time, turnaround time, quantum length, and dispatch length. Longer dispatch times produce longer wait times and longer turnaround times. While inversley, longer quantum times produce shorter wait times and shorter turnaround times. The best scenario seems to be the longest possible quantum length with the shortest possible dispatch time. For quantum times this seems to be mainly due to the quantum being cut short when a process finishes. So there is never any wasted time while in the quantum.

### Extra Analysis
I tried another experiment by increasing the quantum length even more by doubling the previous maximum of 500 up to 1000. The results show even better wait times and turnaround times. Once the quantum becomes large enough that any given task can complete within one quantum, the round-robin algorithm essentially devolves into a FCFS (first come first serve) algorithm.

It seems the case that as the dispatch time increases, quantum length becomes more important for shorter turnaround times (and also shorter wait times). Equally, as the quantum time decreases, the dispatch time becomes more influential.
A simple analysis shows that the difference between quantum lengths of 50 and 500 at a dispatch time of 0 gives roughly 17.8% time improvement (using quantum of 500). On the other side, the improvement of using a quantum time of 500 over 50 with a dispatch time of 25 yields a 47.9% improvement.