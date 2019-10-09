#include "ns_API.h"

#include "needham.h"
#include "rin_wrapper.h"

#include <errno.h>
/* extern function from the original seNS*/
int ns_resolve_address(char *address, int port, ns_abstract_address_t *resolved);
#define NS_MSG_CODE 100

int g_encryption = NS_ENCRYPTION_RIJNDAEL;

// -----------------------------------
//             Client
// -----------------------------------
int run = 1;

typedef struct {
    UT_hash_handle hh;
    char name[NS_IDENTITY_LENGTH];
    char key[NS_KEY_LENGTH];
} identity_t;

identity_t *identities_client;
ns_context_t *context_client;
int g_client_port;

int getport(char *addr)
{
    char *address = strdup(addr);
    strtok(address, ":");
    char *port = strtok(NULL, "");
    int ret = -1;
    if(port)
    {
        ret = atoi(port);
    }
    free(address);
    return ret;
}

void getIP(char *addr, char *IP)
{
    char *address = strdup(addr);
    char *tmpIP = strtok(address, ":");
    strcpy(IP, tmpIP);
    free(address);
}
int store_key_client(char *identity_name, char *key) {
    identity_t *id;
    id = malloc(sizeof(identity_t));
    memset(id, 0, sizeof(identity_t));
    
    memcpy(id->name, identity_name, NS_IDENTITY_LENGTH);
    memcpy(id->key, key, NS_KEY_LENGTH);
    
    HASH_ADD_STR(identities_client, name, id);
    return 0;
}

int get_key_client(char *identity_name, char *key) {
    
    identity_t *id = NULL;
    HASH_FIND_STR(identities_client, identity_name, id);
    
    if(id) {
        memcpy(key, id->key, NS_KEY_LENGTH);
        return 0;
    } else {
        return -1;
    }
}

int event_client(int code) {
    printf("needham-schroeder finished its negotiation with exit code %s.\n",
           ns_state_to_str(code));
    run = 0;
    return 0;
}

void print_identities(identity_t * identities) {
    identity_t *id = NULL;
    int i = 0;
    char tmp_key[NS_KEY_LENGTH+1] = { 0 };
    if(identities) {
        for(id = identities; id != NULL; id = id->hh.next) {
            memcpy(tmp_key, id->key, NS_KEY_LENGTH);
            printf("%d - name : %s, key : %s\n", i, id->name, tmp_key);
            i++;
        }
    } else {
        printf("- no identities stored -\n");
    }
}


int send_to_peer(struct ns_context_t *context, ns_abstract_address_t *addr,
                 uint8_t *data, size_t len) {
    
    int ret = sendto( *((int*) context->app), data, len, MSG_DONTWAIT, &addr->addr.sa, addr->size);
    if (ret < 0)
    {
        perror("sendto");
        
    }
    return ret;
}

int ns_setup_client_default()
{
    g_client_port = DEFAULT_PORT_CLIENT;
    return 0;
}

int ns_setup_client(int port, int encryption)
{
    if (port <= 0)
    {
        port = DEFAULT_PORT_CLIENT;
    }
    g_client_port = port;
    if (encryption > 0)
    {
        g_encryption = encryption;
    }

    return 0;
}

int fd;
int ns_client_negotiate_keys(char *server_address, char *partner_address)
{
    
    int read_bytes;
    char in_buffer[NS_CLIENT_BUFFER_SIZE];
    
    ns_abstract_address_t tmp_addr;
    memset(&tmp_addr, 0, sizeof(ns_abstract_address_t));
    tmp_addr.size = sizeof(tmp_addr.addr);

    fd_set rfds;
    struct timeval timeout;
    char server_IP[50], partner_IP[50];
    
    int sres = 0;
    

    
    int server_port = getport(server_address);//50000;
    if (server_port <= 0) {
        server_port = DEFAULT_PORT_SERVER;
    }
    int partner_port = getport(partner_address);//50010;
    if (partner_port <= 0) {
        partner_port = DEFAULT_PORT_DAEMON;
    }
    getIP(server_address, server_IP);
    getIP(partner_address, partner_IP);
    
    char *client_identity = "example_client";
    char *partner_identity = "example_daemon";
    
    char *key = "0123456789012345";
    
    ns_handler_t handler  = {
        .read = NULL,
        .write = send_to_peer,
        .store_key = store_key_client,
        .get_key = get_key_client,
        .event = event_client
    };
    
    fd = ns_bind_socket(g_client_port, AF_INET);
    if(fd < 0)
        exit(-1);
    
    context_client = ns_initialize_context(&fd, &handler);
    if(!context_client) {
        ns_log_fatal("could not create context.");
        exit(-1);
    }
    
    ns_set_credentials(context_client, client_identity, key);
    
    ns_set_role(context_client, NS_ROLE_CLIENT);
    
    /* Here the key retrieval process starts */
    
    ns_log_info("server: %s, %d | partner: %s, %d, %s", server_IP,
                server_port, partner_IP, partner_port, partner_identity);
    
    ns_get_key(context_client, server_IP, server_port, partner_IP, partner_port,
               partner_identity);
    
    while(context_client->state < NS_STATE_FINISHED && run == 1) {
        
        FD_ZERO(&rfds);
        FD_SET(*((int*) context_client->app), &rfds);
        timeout.tv_sec = NS_RETRANSMIT_TIMEOUT;
        timeout.tv_usec = 0;
        
        sres = select((*((int*) context_client->app))+1, &rfds, 0, 0, &timeout);
        
        if(sres < 0) {
            ns_log_warning("error while waiting for incoming packets: %s", strerror(errno));
            
            /* timeout */
        } else if(sres == 0) {
            ns_retransmit(context_client);
            
            /* new packet arrived, handle it */
        } else {
            read_bytes = recvfrom(fd, in_buffer, sizeof(in_buffer), 0, &tmp_addr.addr.sa, &tmp_addr.size);
            ns_handle_message(context_client, &tmp_addr, in_buffer, read_bytes);
        }
    }
    
    if(context_client->state == NS_STATE_FINISHED) {
        ns_log_info("successfully finished key negotiation.");
    } else {
        ns_log_error("an error occured while negotiating the key: %s", ns_state_to_str(context_client->state));
    }
    
    /* Key process completed, either on success or any error */
    
    print_identities(identities_client);

    //ns_destroy_context(context);
    return 0;
}

int ns_client_send_msg(char *partner_address, char *msg, int msg_len)
{
    char key[NS_KEY_LENGTH];
    char buffer[1 + ns_padded_length(msg_len)];
    int buffer_len= 1 + ns_padded_length(msg_len);
    char msg_padded[ns_padded_length(msg_len)];
    char IP[50];
    char keystr[NS_KEY_LENGTH + 1] = {0};
    ns_abstract_address_t paddr;
    
    if(get_key_client("example_daemon", key) == -1)
    {
        printf("get_key_client failed\n");
        return -1;
    }
    int port = getport(partner_address);
    if (port <= 0)
    {
        port = DEFAULT_PORT_DAEMON;
    }

    getIP(partner_address, IP);
    if (ns_resolve_address(IP, port, &paddr) == -1)
    {
        printf("ns_resolve_address failed\n");
        return -1;
    }
    //printf("send_to_peer, msg= %s\n", msg);
    
    memset(buffer, 0, buffer_len);
    buffer[0] = NS_MSG_CODE;
    memset(msg_padded, 0, sizeof(msg_padded));
    memcpy(msg_padded, msg, msg_len);
    
    //printf("msg_len=%d, ns_padded_fuck=%d\n", msg_len, ns_padded_length(msg_len));
    ns_encrypt_pkcs7((u_char*) key, (u_char*) msg_padded, (u_char*) &buffer[1],
                     ns_padded_length(msg_len), NS_RIN_KEY_LENGTH);
    memcpy(keystr, key, NS_KEY_LENGTH);
    printf("Send msg \"%s\", encrypted using key \"%s\"\n",
           msg, keystr);
    send_to_peer(context_client, &paddr, buffer, buffer_len);
    
    return 0;
}

// -----------------------------------
//             Daemon
// -----------------------------------

identity_t *identities_daemon;
ns_context_t *context_daemon;
int g_daemon_port;

/**
 * store_key may be called more than once, this callback should update the
 * existing identity if it is already present in the data storage.
 */
int store_key_daemon(char *identity_name, char *key) {
    
    identity_t *id;
    HASH_FIND_STR(identities_daemon, identity_name, id);
    
    /* identity not found, insert a new one */
    if(id == NULL) {
        id = malloc(sizeof(identity_t));
        memcpy(id->name, identity_name, NS_IDENTITY_LENGTH);
        memcpy(id->key, key, NS_KEY_LENGTH);
        HASH_ADD_STR(identities_daemon, name, id);
        
        /* identity was already present, update the key */
    } else {
        memcpy(id->key, key, NS_KEY_LENGTH);
    }
    return 0;
}

int get_key_daemon(char *identity_name, char *key) {
    
    identity_t *id = NULL;
    HASH_FIND_STR(identities_daemon, identity_name, id);
    
    if(id) {
        memcpy(key, id->key, NS_KEY_LENGTH);
        return 0;
    } else {
        return -1;
    }
}

int ns_setup_daemon_default()
{
    g_daemon_port = DEFAULT_PORT_DAEMON;
    return 0;
    
}
int ns_setup_daemon(int port, int encryption)
{
    if (port <= 0)
    {
        port = DEFAULT_PORT_DAEMON;
    }
    g_daemon_port = port;
    if (encryption > 0)
    {
        g_encryption = encryption;
    }

    return 0;
}

void ns_daemon_loop_simple()
{
    char in_buffer[NS_DAEMON_BUFFER_SIZE];
    int fd, read_bytes;
    
    ns_abstract_address_t tmp_addr;
    memset(&tmp_addr, 0, sizeof(ns_abstract_address_t));
    tmp_addr.size = sizeof(tmp_addr.addr);
    

    char *key = "1111111111222222";
    char *identity = "example_daemon";
    
    ns_handler_t handler  = {
        .read = NULL,
        .write = send_to_peer,
        .store_key = store_key_daemon,
        .get_key = get_key_daemon,
        .event = NULL
    };
    
    fd = ns_bind_socket(g_daemon_port, AF_INET6);
    
    context_daemon = ns_initialize_context(&fd, &handler);
    if(!context_daemon) {
        ns_log_fatal("could not create context.");
        exit(-1);
    }
    
    ns_set_credentials(context_daemon, identity, key);
    
    ns_set_role(context_daemon, NS_ROLE_DAEMON);
    
    ns_log_info("listening on port %d", g_daemon_port);
    
    while(1) {
        read_bytes = recvfrom(fd, in_buffer, sizeof(in_buffer), 0, &tmp_addr.addr.sa, &tmp_addr.size);
        ns_handle_message(context_daemon, &tmp_addr, in_buffer, read_bytes);
    }
    
    ns_destroy_context(context_daemon);

}

char in_buffer[70000];
char decrypted[70000];
void ns_daemon_loop(void (*msg_handler)(char *, int ))
{
    
    int fd, read_bytes;
    char key_dc[NS_KEY_LENGTH];
    
    ns_abstract_address_t tmp_addr;
    memset(&tmp_addr, 0, sizeof(ns_abstract_address_t));
    tmp_addr.size = sizeof(tmp_addr.addr);
    
    
    char *key = "1111111111222222";
    char *identity = "example_daemon";
    
    ns_handler_t handler  = {
        .read = NULL,
        .write = send_to_peer,
        .store_key = store_key_daemon,
        .get_key = get_key_daemon,
        .event = NULL
    };
    
    fd = ns_bind_socket(g_daemon_port, AF_INET6);
    
    context_daemon = ns_initialize_context(&fd, &handler);
    if(!context_daemon) {
        ns_log_fatal("could not create context.");
        exit(-1);
    }
    
    ns_set_credentials(context_daemon, identity, key);
    
    ns_set_role(context_daemon, NS_ROLE_DAEMON);
    
    ns_log_info("listening on port %d", g_daemon_port);
    
    while(1) {
        read_bytes = recvfrom(fd, in_buffer, sizeof(in_buffer), 0, &tmp_addr.addr.sa, &tmp_addr.size);
        
        //ns_log_info("in_buffer= %d\n", (int)in_buffer[0]);
        if(in_buffer[0] == NS_MSG_CODE)
        {
            //printf("yah\n");
            if(get_key_daemon("example_client", key_dc) == -1)
            {
                printf("get_key_daemon failed\n");
                continue;
            }
            //printf("readbytes=%d\n", read_bytes);
            ns_decrypt((u_char*) key_dc, (u_char*) &in_buffer[1], (u_char*) decrypted,
                       read_bytes-1, NS_RIN_KEY_LENGTH);
            decrypted[read_bytes-1] = 0;
            msg_handler(decrypted, read_bytes - 1);
        }
        else
            ns_handle_message(context_daemon, &tmp_addr, in_buffer, read_bytes);
    }
    
    ns_destroy_context(context_daemon);
    
    
    
}


// -----------------------------------
//             server
// -----------------------------------

identity_t *identities_server;
ns_context_t *context_server;
int g_server_port;

/* ---------------------------- Identity Storage --------------------------- */

/**
 * Callback function to get a key for \p identity_name. This function could
 * get identity-key-pairs from a database for example. Keys must have the length
 * of NS_RIN_KEY_LENGTH bytes.
 *
 * In this example the server knows the identities "example_client" and
 * "example_daemon" and their corresponding keys. A return value of -1 indicates
 * an unknown \p identity_name.
 */
int get_key_server(char *identity_name, char *key) {
    
    if(strcmp(identity_name, "example_client") == 0) {
        char *client_key = "0123456789012345";
        memcpy(key, client_key, NS_RIN_KEY_LENGTH);
        return 0;
        
    } else if(strcmp(identity_name, "example_daemon") == 0) {
        char *daemon_key = "1111111111222222";
        memcpy(key, daemon_key, NS_RIN_KEY_LENGTH);
        return 0;
        
    } else if(strcmp(identity_name, "smartobject-1") == 0) {
        char *daemon_key = "1111111111222222";
        memcpy(key, daemon_key, NS_RIN_KEY_LENGTH);
        return 0;
        
    } else if(strcmp(identity_name, "bender") == 0) {
        char *daemon_key = "1234567890123456";
        memcpy(key, daemon_key, NS_RIN_KEY_LENGTH);
        return 0;
        
    } else if(strcmp(identity_name, "rd_12345") == 0) {
        char *daemon_key = "1111111111222222";
        memcpy(key, daemon_key, NS_RIN_KEY_LENGTH);
        return 0;
        
    } else {
        return -1;
    }
}

int ns_setup_server_default()
{
    g_server_port = DEFAULT_PORT_SERVER;
    return 0;
    
}

int ns_setup_server(int port, int encryption)
{
    if (port <= 0)
    {
        port = DEFAULT_PORT_SERVER;
    }
    g_server_port = port;
    if (encryption > 0)
    {
        g_encryption = encryption;
    }

    return 0;
    
}

void ns_server_loop()
{
    char in_buffer[NS_SERVER_BUFFER_SIZE];
    int fd, read_bytes;

    
    ns_abstract_address_t tmp_addr;
    memset(&tmp_addr, 0, sizeof(ns_abstract_address_t));
    tmp_addr.size = sizeof(tmp_addr.addr);
    
    ns_handler_t handler  = {
        .read = NULL,
        .write = send_to_peer,
        .store_key = NULL,
        .get_key = get_key_server,
        .event = NULL
    };
    
    fd = ns_bind_socket(g_server_port, AF_INET6);
    if(fd < 0)
        exit(-1);
    
    context_server = ns_initialize_context(&fd, &handler);
    if(!context_server) {
        ns_log_fatal("could not create context.");
        exit(-1);
    }
    
    ns_set_role(context_server, NS_ROLE_SERVER);
    
    ns_log_info("Server running (port: %d), waiting for key requests.", g_server_port);
    
    while(1) {
        read_bytes = recvfrom(fd, in_buffer, sizeof(in_buffer), 0, &tmp_addr.addr.sa, &tmp_addr.size);
        ns_handle_message(context_server, &tmp_addr, in_buffer, read_bytes);
    }
    
    ns_destroy_context(context_server);
    
}
