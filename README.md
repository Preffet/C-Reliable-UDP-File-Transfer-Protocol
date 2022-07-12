![Badge tracking last commit](https://img.shields.io/github/last-commit/Preffet/C-Reliable-UDP-File-Transfer-Protocol?color=%23013987)
![Badge tracking number of files](https://img.shields.io/github/directory-file-count/Preffet/C-Reliable-UDP-File-Transfer-Protocol?color=%23016a87)
![Badge tracking code size](https://img.shields.io/github/languages/code-size/Preffet/C-Reliable-UDP-File-Transfer-Protocol?color=%2301877a)
![Badge tracking repo size](https://img.shields.io/github/repo-size/Preffet/C-Reliable-UDP-File-Transfer-Protocol?color=%23013987)
![updated](https://user-images.githubusercontent.com/84241003/178457017-7126a06b-e0c1-484e-b0a8-c94b7f98ab2b.gif)


---------------------------------------------------------------------------

**The purpose of this project is to demonstrate two file transfer protocols:**
- Reliable file transfer (RFT) with positive acknowledgement in the absence of errors 
- Reliable file transfer (RFT) with positive acknowledgement and retransmission  – an example 
of a PAR protocol

---------------------------------------------------------------------------

The RFT application comprises client and server applications:
- rft_client
- rft_server

The server has to be ran in one terminal window and the client in another 
terminal window either on the same host (in which case the server address
will be 127.0.0.1) or a different host (in which case the server address
will be for an external IP interface).


### Ways to run the project

**1) Using the provided shell scripts**
The following two shell scripts are provided as convenient ways to the
run the applications:

    - run-server.sh to compile and run the rft_server
        (which also compiles the rft_client)
    
    - run-client.sh to compile and run the rft-client
    
By default, the run-server.sh script will compile the applications and
start the server on port 20333, and the run-client.sh script will start
the client and attempt to send the file in660.txt to the server for
output on the server side in the file ./out/out.txt. The normal transfer
protocol is used by default.

That is, if you do:

    ./run-server.sh 
    
in one terminal window, and:

    ./run-client.sh
    
in another terminal window on the same host, then provided the
rft_client_util library has been implemented, the client will transfer
the file in660.txt to the server using the send_file_normal function.

Both scripts have options to allow you to specify a different port
and/or server address and to transfer different files using either the
normal transfer protocol without retransmission or the protocol with
timeout and retransmission.

To see the server script help and usage message, enter the command:

    ./run-server.sh -h
    
To see the client script help and usage message, enter the command:

    ./run-client.sh -h
    
When a transfer is completed you can use the [diff](https://man7.org/linux/man-pages/man1/diff.1.html)
command to check that the client-side input file is the same as the server-side output file.

TIP: if you want to test that you have correctly implemented reaching
the limit of consecutive retries, just run the client with input file
in660.txt with a loss probability of 0.5 or more without running the
server. That is, enter the command:

    ./run-client.sh 660 0.5
    
which should generate enough message loss to reach the limit of
consecutive retries (provided there is no server running and therefore
no ACKs at all).


**2) Running the rft_client and rft_server applications directly**

You do not have to use the shell scripts. You can run the rft
applications directly.

To do this, enter the command:

    make 
    
to build the applications.

Then, assuming compilation succeeds, enter the command:

    ./rft_server 
    
to see the options for running the server application.

And:

    ./rft_client
    
to see the options for running the client application


### Ways to test the project

    A test program is provided to test a subset of the rft_client_util
    library in isolation. The test program is test_rft_client_util. This
    does not test communication between client and server. It just tests
    some of the functions of the rft_client_util library.

    The simplest way to build and run the test program is to enter the
    following command:

        make test

    this will compile the test program and run it. It uses the same test
    framework as was used for assignment 1 and output is similar. Currently,
    tests are provided for init_protocol, init_segment and set_udp_socket
    functions.

---------------------------------------------------------------------------
2. AN OVERVIEW OF INPUT FILES

The following input files are provided for you:

    in0.txt
    in1.txt
    in17.txt
    in34.txt
    in35.txt
    in36.txt
    in350.txt
    in660.txt
    
The number that follows "in" in the filename is the size of the file in
bytes.

The sizes have been chosen deliberately to test the following file
transfer conditions:

    - zero sized file
    - boundary condition file sizes, e.g. one more than zero and sizes  
      one off the significant file data in a data segment payload
    - multiples of significant file data size
    - a larger file to test random data segment loss
    
When testing your solutions it is a good idea to try each of the input
files to check that the solution works correctly for the different file
sizes.

---------------------------------------------------------------------------
3. AN OVERVIEW OF EXAMPLE PROTOCOL LOG OUTPUT FILES

The following files provide examples of the log output from the
rft_client and rft_server:

    README-client-normal-tfr-out.txt:
        output from the client for successful execution of a normal 
        transfer
    README-server-normal-tfr-out.txt:
        output from the server for successful execution of a normal 
        transfer
        
    README-client-normal-tfr-err.txt:
        output from the client when there is an error in a normal 
        transfer

    README-client-timeout-tfr-out.txt
        output from the client for successful execution of a transfer 
        with timeout
    README-server-timeout-tfr-out.txt:
        output from the server for successful execution of a normal 
        transfer

    README-client-timeout-tfr-err.txt
        output from the client when there is an error in a transfer 
        with timeout

The above files are intended to give you an idea of the sort of output
to expect during protocol execution, provided you have used logging and
exit with error correctly.

---------------------------------------------------------------------------




