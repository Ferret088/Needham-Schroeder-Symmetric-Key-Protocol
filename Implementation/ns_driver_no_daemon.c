/*! \file ns_driver.c
 \brief A simple driver program.
 */
#include "ns_API.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*! \fn void daemon_process()
    \brief The daemon process
 */
void daemon_process()
{
    if ((execlp("xterm", "xterm", "-e", "./ns_daemon", 
                (char *) 0))
        < 0)  {
        perror("execlp failed");
        exit(-1);
    }
}

/*! \fn void server_process()
 \brief The server process
 */
void server_process()
{
    if ((execlp("xterm", "xterm", "-e", "./ns_server", 
                (char *) 0))
        < 0)  {
        perror("execlp failed");
        exit(-1);
    }
}

/*! \fn void client_process()
 \brief The client process.
 */
void client_process()
{
    if ((execlp("xterm", "xterm", "-e", "./ns_client",
                (char *) 0))
        < 0)  {
        perror("execlp failed");
        exit(-1);
    }
    
}

/*! \fn int main()
    \brief Driver program. Creates three processes, i.e. server, client and daemon.
 */
int main()
{
    pid_t childpid, childpid2, childpid3;
    int childStatus;
    
    printf("Driver program\n");
    
    /* create new process*/
    childpid = fork();
    if(childpid >= 0)
    {/*fork succeed*/
        if (childpid == 0)
        {
            /*child process, set it as Daemon*/
            //daemon_process();
            
        } else {
            childpid2 = fork();
            if (childpid2 == 0)
            {
                /*child process 2, set it as Server*/
                server_process();
            } else
            {
                childpid3 = fork();
                if(childpid3 == 0)
                {
                    /*child process 2, process, set it as Client*/
                    client_process();
                }
                else
                {
                    waitpid(childpid, &childStatus, 0);
                    waitpid(childpid2, &childStatus, 0);
                    waitpid(childpid3, &childStatus, 0);
                }
                
            }
        }
    } else
    {
        perror("fork");
        return -1;
    }
    return 0;
}

