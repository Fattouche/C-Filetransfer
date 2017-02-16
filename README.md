# UDP Web Server with MultiThreading
Alex Fattouche

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

