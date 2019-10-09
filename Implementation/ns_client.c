#include "needham.h"
#include "stdio.h"
#include "stdlib.h"
#include "ns_API.h"
#include <unistd.h>
#include "time.h"

int main(int argc, char **argv) {

    char msg[] = "Hi, I'm the client";
    char server_address[] = "127.0.0.1:60002";
    char daemon_address[] = "127.0.0.1:60001";

    //for performance measurement
    clock_t begin, end;
    double time_spent;
    
    printf("Client:argc= %d\n", argc);
    
    

    sleep(2);
    begin = clock();
    if (argc > 1 && strcmp("rijndael", argv[1]) == 0)
    {
        printf("ENCRYPTION: RIJNDAEL\n");
        ns_setup_client(60000, NS_ENCRYPTION_RIJNDAEL);
    }else
    {
        printf("ENCRYPTION: BLOWFISH\n");
        ns_setup_client(60000, NS_ENCRYPTION_BLOWFISH);
    }
    ns_client_negotiate_keys(server_address, daemon_address);
    /* This is not supported in the original library*/
    ns_client_send_msg(daemon_address, msg, strlen(msg));
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    
    printf("Time spented: %lf ms\n", time_spent * 1000);
    getchar();
}
