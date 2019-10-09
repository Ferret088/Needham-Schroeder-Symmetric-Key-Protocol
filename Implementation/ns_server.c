#include "needham.h"
#include "stdio.h"
#include "stdlib.h"
#include "ns_API.h"


int main(int argc, char **argv) {
    printf("Server:argc= %d\n", argc);
   
    if (argc > 1 && strcmp("rijndael", argv[1]) == 0)
    {
        printf("ENCRYPTION: RIJNDAEL\n");
        ns_setup_server(60002, NS_ENCRYPTION_RIJNDAEL);
    }else
    {
        printf("ENCRYPTION: BLOWFISH\n");
        ns_setup_server(60002, NS_ENCRYPTION_BLOWFISH);

    }
    ns_server_loop();
}
