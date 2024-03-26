#include "udp.h"

static pthread_t tid;
static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;

static bool is_initialized = false;

static void *udp_listener(void *args);

void udp_init()
{
    is_initialized = true;
    if(pthread_create(&tid, NULL, &udp_listener,NULL) != 0) {
        perror("ERROR: udp_init: Could not create thread.\n");
        exit(1);
    }
}

void udp_cleanup()
{
    if(pthread_join(tid, NULL) != 0) {
        perror("ERROR: udp_cleanup: Error joining thread.\n");
        exit(1);
    }
    is_initialized = false;
}

bool udp_isInitialized() {
    return is_initialized;
}

static void * udp_listener(void *args) 
{
    (void) args;
    char lastMessage[BUF_LENGTH] = {0};

    char buffer[BUF_LENGTH] = {0};
    struct sockaddr_in sin = {0};
    struct sockaddr_in client = {0};

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("failed to create socket\n");
        exit(1);
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(12345);
    sin.sin_addr.s_addr = INADDR_ANY;

    int rc = bind(sockfd, (const struct sockaddr *)&sin,
                    sizeof(sin));

    if (rc == -1)
    {
        perror("failed to bind\n");
        close(sockfd);
        exit(1);
    }

    int numPackets;

    socklen_t len = sizeof(client);
    while (is_initialized) {

        int n = recvfrom(sockfd, (char *)buffer, BUF_LENGTH, MSG_WAITALL,
                         (struct sockaddr *)&client, &len);
        buffer[n] = '\n';

        // main thread logic, input case handling
        // errant inputs will put out the 'help' prompt.
        if(buffer[0] - '\n' == 0) {
            snprintf(buffer,strlen(lastMessage)+1,lastMessage);
        }

        char msg[50] = {0};
        int written;
        int offset = 0;

        
        // Stop condition for the entire program.
        if(strncmp(buffer,"stop",4) == 0) {
            char *message = "Stopping...\n";
            snprintf(msg,strlen(message)+1,message);
            printf("%s",message);
            is_initialized = false;
        }
        // Help message
        else if(strncmp(buffer,"help",4) == 0 || strncmp(buffer,"?",1) == 0){

            char *message = 
            "Accepted command examples:"
            "\nstop       -- cause the server program to end."
            "\n<enter>    -- repeat last command.\n";

            snprintf(msg,strlen(message)+1,message);

        }

        else {
            char *message = "Please type 'help' or '?' for list of accepted commands.\n";

            snprintf(msg,strlen(message)+1,message);
        }
        
        // split msg into packets and send over UDP.
        numPackets = (double) strlen(msg) / (double) MAX_PACKET_SIZE +1;
        int start = 0;
        int end = 0;

        for(int i = 0; i < numPackets; i++) {

            for(size_t j = 0; j < MAX_PACKET_SIZE; j++) {

                if(!isalnum(msg[j+start]) || msg[j+start] != '.') {

                    end = j+start;
                }
            }

            sendto(sockfd, (const char *) msg+start, end, MSG_CONFIRM,
                (const struct sockaddr *)&client, sizeof(client));

            start += end;
        }

        snprintf(lastMessage, strlen(buffer)+1, buffer);

    }
    close(sockfd);
    return NULL;
}