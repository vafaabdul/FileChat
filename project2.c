#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <signal.h>

volatile sig_atomic_t flag=0;
int fd_dest=0;
FILE *stream;

//usage
usage(){

    printf("Usage : ./project2 source destination \n");
}

//cleanUp
void cleanUp(){

  if(stream != NULL){

    fclose(stream);
  }

  if(fd_dest>0){

    close(fd_dest);
  }

}

//alarm signal handler
void alarmHandler(int sig){
	flag=1;
}

//handling interupt signal
void interuptHandler(int sig){
  exit(EXIT_SUCCESS);
}

//function to write oto console
void writeToConsole(FILE *stream){

  char* out_line=NULL;
  ssize_t nread;
  char* in_line=NULL;
  size_t capacity=0;
  int line_size=0;
  do{
      line_size= getline(&in_line, &capacity, stream);
      clearerr(stream);
      if(line_size > 0){
        printf("%s",in_line);
      }
      else{

          flag=0;
          break;
      }

  }while(line_size>0);
  free(in_line);
}

//function to read from console
void readFromConsole(int fd_dest){

  char* in_line=NULL;
  size_t read=0;
  size_t capacity=0;

  alarm(3);
  read=getline(&in_line,&capacity,stdin);
  clearerr(stdin);
  alarm(0);

  if(read !=-1){

//get the cureent timestamp
    char * now_time="";
    time_t curtime;
    time(&curtime);
    now_time=ctime(&curtime);
    size_t ln = strlen(now_time) - 1;

//remove the new line char
    if (now_time[ln] == '\n')
      now_time[ln] = '\0';

    strcat(now_time,",");
    size_t nread=strlen(now_time);
  //  strcat(now_time,in_line);

//write timestamp to the file
    write(fd_dest, now_time, nread);
//write data read from console to file
    char * out_ptr = in_line;
    ssize_t nwritten;

    do {
        nwritten = write(fd_dest, out_ptr, read);

        if (nwritten >= 0)
        {
            read -= nwritten;
            out_ptr += nwritten;
        }
        else if (errno != EINTR)
        {
            if(fd_dest >=0)
            close(fd_dest);
            perror("output file");
            exit(1);
        }
    } while (read > 0);

  }

  free(in_line);
}

//main function
int main(int argc,char * argv[]){

    char * input=NULL;
    char * output=NULL;
    int fd_dest=0;
    FILE *stream;

    if(argc<3)
    {
        usage();
        return 0;
    }


    struct sigaction act;
    act.sa_handler=interuptHandler;

    if(sigaction(SIGINT,&act,NULL)){
      		perror("sigaction");
      		exit(1);
    }

    struct sigaction action;
  	action.sa_handler=alarmHandler;

  	if(sigaction(SIGALRM,&action,NULL)){
  		perror("sigaction");
  		exit(1);
  	}


    //un set SA_RESTART
    if(siginterrupt(SIGALRM,1)){

        perror("siginterrupt");
    }


    input=argv[1];
    output=argv[2];

    //open input file
    stream = fopen(input, "r");

    if (stream == NULL){
      perror(input);
      exit(EXIT_FAILURE);

    }

    //open or create output file
    fd_dest = open(output, O_WRONLY | O_CREAT | O_APPEND, 0666);

    if (fd_dest < 0){
        perror(output);
        exit(1);
    }
    //register cleanup method
    int myFlag=atexit(cleanUp);

    while(1){

      //call for reading from console
      if(flag==0){
        readFromConsole(fd_dest);
      }
      //call for writing to file
      if(flag==1){
        writeToConsole(stream);
      }

    }

}
