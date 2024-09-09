# Project of Laboratorio II

## This is my personal implementation of a distributed library system.

The project is divided into 2 main parts: 

- Server: it is a multitreaded system capable of managing multiple connections simultaneously.
- Client: is a single threaded program that queries the server based on certain inputs

There are also four libraries divided as follows:
- book.c: contains all the functions useful for the correct use of the "single_book" data structure.
- linkedList.c: it's my personal implementation of thread safe linked list.
- queue.c: it is a thread safe queue implementation provided by the teacher.
- util.c: contains all utility functions,

## Usage

The project offers the possibility to start servers and clients individually or start a test session, For both methods of use, a preliminary action must be carried out:

```bash
~/project_dir$ make prepfile
```

Subsequently:
- to start server and client individually:
    
    (a server will be started, then after waiting 1 second a client will be started)

    -   ```bash
        ~/project_dir$ make runsingle
        ```
- to start the test session: 

    -   ```bash
        ~/project_dir$ make test
        ``` 

The servers end automatically but, if they do not stop, you can end them with a SIGKILL with the command: 
```bash
~/project_dir$ make hardstop
```
    
        



