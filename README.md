# Needham-Schroeder-Symmetric-Key-Protocol

## 1. Introduction
This c implementation of the Needham Schroeder Symmetric Key Protocol is based on the previous work done by Andreas Bender. This implementation includes timestamp to defeat the replay attack. 

## 2.Design
The design includes APIs and a simple driver program. To understand the design, it's necessary to understand how the protocol works. Below is the message flow:
<pre>
A->S: A,B,Na
S->A: {Na, Kab, B, {Kab, A, T}Kbs}Kas
A->B: {Kab, A, T}Kbs
B->A: {Nb}Kab
A->B: {Nb-1}Kab
</pre>
<br>
There are 3 roles here, A, B and S. We call A as client, B as daemon and S as server. There are APIs for each role. Take setup for example, there are ns_setup_client, ns_setup_daemon, ns_setup_server for each role. In the setup, data structures for storing keys are created and network related operations, such as Bind, Listen are performed. Each role can pass port number and encryption method to setup, otherwise, the default is used. <br>

After setup, daemon and server can simply call ns_daemon_loop_simple and ns_server_loop respectively to enter default message handling loop. If daemon needs to do additional message handling, a callback can be passed to ns_daemon_loop. The client needs to call ns_client_negotiate_keys to initiate the key negotiation. After that, the client can call ns_client_send_msg to send encrypted messages to daemon, using the negotiated share key. <br>

The driver program serves as a simple example of using the API and also a simple test case. It creates 3 processes, one for the server, one for the daemon and one for the client. APIs are called by the driver program to perform key negotiation. After that, A simple hello message is sent from the client to the daemon. <br>


## 3 How to use:
<pre>
1. Go to the project directory and issue a make. This step will generate the driver programs in the subdirectory Implementation.
	EX: make
2. CD to the sub subdirectory Implementation. (NOTE: This is required because driver use relative path to call server, daemon and client)
	EX: cd Implementation
3. Run the driver
	EX: ./ns_driver_blowfish
	       ./ns_driver_rijndael
	       ./ns_driver_no_server
                    ./ns_driver_no_daemon
 </pre>
