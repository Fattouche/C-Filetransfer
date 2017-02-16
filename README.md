# UDP Web Server with MultiThreading
Alex Fattouche V00829296
lab: B03 - Dawood Sajjadi

### Code Design
To compile: make sws
To run without multithreading: make run

To compile client(for threading): make client
To run with multithreading: make runthread
To run client(for threading):make runclient

The code was designed so that pthreads could be easily swapped in and out
to ensure testing was the same. Inside the main function we handle requests
on our server as well as input from the user using select. Once we get
a request from the server we make a struct with information, make a pthread id
and then create the thread with that information passed in. Inside the function call
we handle parsing data, deciding if the request was 400,404 or 200 and then sending
information back. Lastly we log the request and return a void pointer.

### Code Structure
From top to bottom of the C file the code structure is as follows:

1. Create Struct
2. sendBack - Function for sending data back to the user  
3. HandleRequest - The core of parsing and interacting with user
4. main - takes in port number and directory as parameters, watches for user/socket data


### Bonus
I used my basic knowledge of pthreads to implement a multi threaded udp server. The main
way that I tested its functionality was through load testing, simply running my shell
script several times with 'nohup' and '&' flags set. I have also created a test inside the
C file that shows that the thread is working while the main loop is still running.
The test outputs from main into the thread_out.txt file before it outputs
from the thread, which could only happen if it was multithreaded. In addition,
the threads are utilized so that they are dealt with in cronological order.

To show that the multithreading is utilizing multiple cores, you can run the ./udp_client
and then run 'top' followed by pressing 1 in the terminal. When we look at the 'wa'
column, we can see that all of the cpu's are being used when multithreading happens
and that not all of them are being used without multithreading. Although in both
cases the main thread is being used more than all the other ones because it is handling
the requests.

Benefits of Multithreading:

1. Many concurrent operations improves throughput
2. Makes use of multiple processors for computation(can see this with my udp_client.c file)
3. Improves the responsiveness of the server because we should always have available threads
4. Allows for a simple way for us to exit gracefully because the pthread_join function call
will wait untill all threads are finished computing.
5. Decreases overall wait time for clients
