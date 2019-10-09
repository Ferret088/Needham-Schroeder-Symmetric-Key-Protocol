#ifndef _NS_API_H_
#define _NS_API_H_
#include "rin_wrapper.h"

/*! \mainpage Needham Schroeder Symmetric Key Protocol
    \section Introduction
    This is a C implementation of the Needham Schroeder Symmetric Key Protocol,
    based on the previous work [seNS](https://github.com/abender/needham-schroeder )
    done by Andreas Bender. This implementation includes timpstamp to defeat the
    [replay attack](http://en.wikipedia.org/wiki/Needham-Schroeder_protocol ) .
    The project is going to extend seNS in the following ways:
 - Implement tests
 - Implement additional encryption methods
 - Transmission error detection
 - Check cleanups
 - Implement easy-to-use APIs
 - Fix bugs

    \section Design
    The design includes APIs and a simple driver program. To understand the design, 
    it's necessary to understand how the protocol works. Below is the message flow:\n
 - A->S: A,B,Na\n
 - S->A: {Na, Kab, B, {Kab, A, T}Kbs}Kas\n
 - A->B: {Kab, A, T}Kbs\n
 - B->A: {Nb}Kab\n
 - A->B: {Nb-1}Kab
.
    
 There are 3 roles here, A, B and S. We call A as client, B as daemon and S as
 server. There are APIs for each role, take setup for example, there are
 ns_setup_client, ns_setup_daemon, ns_setup_server for each role. In the setup,
 data structures for storing keys are created and network related operations, such
 as Bind,Listen are performed. Each role can pass port number and encryption
 method to setup, otherwise, the default is used.\n
 After setup, daemon and server can simply call ns_daemon_loop_simple and
 ns_server_loop respectively to enter default message handling loop. If daemon needs
 to do additional message handling, a callback can be passed to ns_daemon_loop.
 The client needs to call ns_client_negotiate_keys to initiate the key negotiation.
 After that, the client can call ns_client_send_msg to send encrypted messages to 
 daemon, using the negotiated share key.\n
 The driver program serves as a simple example of using the API and also a simple
 test case. It creates 3 processes, one for the server, one for the daemon and one
 for the client. APIs are called by the driver program to perform key negotiation. 
 After that, A simple hello message is sent from the client to the daemon. Additional 
 test/driver will be implemented later.
 */

/*! \file ns_API.h
    \brief An easy to use API of Needham-Schroeder Protocol:\n
    A->S: A,B,Na\n
    S->A: {Na, Kab, B, {Kab, A, T}Kbs}Kas\n
    A->B: {Kab, A, T}Kbs\n
    B->A: {Nb}Kab\n
    A->B: {Nb-1}Kab\n
 */

/*! \fn int ns_setup_client_default()
 \brief Setup the Client process. Bind to the default port. Use default encryption
 \return On success, zero is returned. On error, -1 is returned.
 */
int ns_setup_client_default();

/*! \fn int ns_setup_client(int port, int encryption)
    \brief Setup the Client process. Bind the port. etc.
    \param port The port bind to client. Default port is used when this parameter is 0.
    \param encryption Specifies the encryption method used in the protocol. Default encryption is used when this parameter is 0.
    \return On success, zero is returned. On error, -1 is returned.
 */
int ns_setup_client(int port, int encryption);

/*! \fn int ns_client_negotiate_keys(char *server_address, char *partner_address)
    \brief Initiate the key negotiation.\nCalled by client.\n
    A->S: A,B,Na\n
    S->A: {Na, Kab, B, {Kab, A, T}Kbs}Kas\n
    A->B: {Kab, A, T}Kbs\n
    B->A: {Nb}Kab\n
    A->B: {Nb-1}Kab\n
    \param server_address The IP address of the server.
    \param partner_address The IP address of the partner(B).
    \return On success, zero is returned. On error, -1 is returned.
 */
int ns_client_negotiate_keys(char *server_address, char *partner_address);

/*! \fn int ns_client_send_msg(char *partner_address, char *msg, int msg_len)
 \brief Send a message to partner_address.\n
 Called by client.\n
 \param partner_address The IP address of the destination of the message.
 \param msg Message buffer to send.
 \param msg_len The length of the message buffer. 
 \return On success, zero is returned. On error, -1 is returned.
 */
int ns_client_send_msg(char *partner_address, char *msg, int msg_len);

/*! \fn int ns_setup_daemon_default()
 \brief Setup the Daemon process. Bind to the default port.
 \return On success, zero is returned. On error, -1 is returned.
 */
int ns_setup_daemon_default();

/*! \fn int ns_setup_daemon(int port, int encryption)
 \brief Setup the Daemon process. Bind the port. etc.
 \param port The port to bind to Daemon.Default port is used when this parameter is 0.
 \param encryption Specifies the encryption method used in the protocol. Default encryption is used when this parameter is 0.
 \return On success, zero is returned. On error, -1 is returned.
 */
int ns_setup_daemon(int port, int encryption);

/*! \fn void ns_daemon_loop_simple()
    \brief Enters the message handling loop, when a message arrives, simply print it out.\n Called by daemon program, i.e. B.
 */
void ns_daemon_loop_simple();

/*! \fn void ns_daemon_loop(void *msg_handler(char *, int ))
    \brief Enters the message handling loop, when a message(except key negotiation messages)arrives, the msg_handler is called with the message content and length passed as parameters.\n Called by daemon program, i.e. B.
    \param msg_handler the user defined message handler.
 */
void ns_daemon_loop(void (*msg_handler)(char *, int ));


/*! \fn int ns_setup_server_default()
 \brief Setup the Server process. Bind to the default port.
 \return On success, zero is returned. On error, -1 is returned.
 */
int ns_setup_server_default();

/*! \fn int ns_setup_server(int port, int encryption)
 \brief Setup the Server process. Bind the port. etc.
 \param port The port to bind to Server. Default port is used when this parameter is 0.
 \param encryption Specifies the encryption method used in the protocol. Default encryption is used when this parameter is 0.
 \return On success, zero is returned. On error, -1 is returned.
 */
int ns_setup_server(int port, int encryption);

/*! \fn void ns_server_loop()
    \brief Server standard task. Called by server which handles only key negotiation messages.
 
*/
void ns_server_loop();

#endif
