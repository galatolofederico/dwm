#include <stdio.h> 
#include <stdlib.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <signal.h>
#include <string.h>

typedef struct MsgBuf MsgBuf;
struct MsgBuf {
    long type;
    char text[255];
};

int main(int argc, char **argv) 
{ 
    if(argc != 3){
        printf("Usage: dwm-msg <pid> <message>\n");
        exit(0);
    }
    int pid = atoi(argv[1]);
    char *text = argv[2];

    key_t key; 
    int msgid; 
    MsgBuf message;
    
    message.type = 1;
    strcpy(message.text, text);

    key = ftok("/tmp/dwm.ipc", 1);
  
    msgid = msgget(key, 0666 | IPC_CREAT); 
    msgsnd(msgid, &message, 255, 0);

    kill(pid, SIGUSR1);
} 