/*! \file ns_driver.c
 \brief A simple driver program.
 */
#include "ns_API.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/*! \fn void daemon_process()
    \brief The daemon process
 */
void daemon_process()
{
    ns_setup_daemon_default();
    /*Enters message handling loop*/
    ns_daemon_loop_simple();
}

/*! \fn void server_process()
 \brief The server process
 */
void server_process()
{
    ns_setup_server_default();
    /*Enters message handling loop*/
    ns_server_loop();
}

/*! \fn void client_process()
 \brief The client process. 
 */
void client_process()
{
    char msg[] = "Hi, I'm the client";
    char server_address[] = "127.0.0.1";
    char daemon_address[] = "127.0.0.1";
    
    
    /*sleep a few seconds, to wait for server and daemon*/
    sleep(2);
    ns_setup_client_default();
    /*use local address, use default ports*/
    ns_client_negotiate_keys(server_address, daemon_address);
    ns_client_send_msg(daemon_address, msg, strlen(msg));
    
}

/*! \fn int main()
    \brief Driver program. Creates three processes, i.e. server, client and daemon.
 */
int main()
{
    pid_t childpid, childpid2;
    
    printf("Driver program\n");
    
    /* create new process*/
    childpid = fork();
    if(chldpid >= 0)
    {/*fork succeed*/
        if (childpid == 0)
        {
            /*child process, set it as Daemon*/
            daemon_process();
            
        } else {
            childpid2 = fork();
            if (childpid2 == 0)
            {
                /*child process 2, set it as Server*/
                server_process();
            } else
            {
                /*parent process, set it as Client*/
                client_process();
            }
        }
    } else
    {
        perror("fork");
        return -1;
    }
    return 0;
}

