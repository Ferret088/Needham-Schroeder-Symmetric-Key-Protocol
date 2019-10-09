#include "needham.h"
#include "ns_API.h"

#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>

//void ns_setup_daemon_default();

void msg_handler(char * c, int i)
{
    printf("Receive encryted msg: \"%s\"\n", c);
}

int main(int argc, char **argv) {
    printf("Daemon:argc= %d\n", argc);
  
    if (argc > 1 && strcmp("rijndael", argv[1]) == 0)
    {
        printf("ENCRYPTION: RIJNDAEL\n");
        ns_setup_daemon(60001, NS_ENCRYPTION_RIJNDAEL);

    }else
    {
        printf("ENCRYPTION: BLOWFISH\n");
        ns_setup_daemon(60001, NS_ENCRYPTION_BLOWFISH);

        
    }
    ns_daemon_loop(&msg_handler);
    return 0;
}
