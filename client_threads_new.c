#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define PORT 2908

extern int errno;

int port;

char raspuns[100];
 int exited = 0;
 int check_if_answered = 0;


//functia pentru threadul care asteapta input de la utilizator
void *Reading()
 {
    char buf[100];
    scanf("%79s[^\n]", buf); 
    check_if_answered = sscanf(buf, "%s", raspuns);

    
    fflush(stdin);

    pthread_exit(NULL);
 }

//functia pentru threadul folosit drept timer la fiecare runda de intrebari
 void *Timer()
 {
    sleep(10);
    exited = 1;

    pthread_exit(NULL);
 }

int main (int argc, char *argv[])
{
  int sd;			
  struct sockaddr_in server;	
  

  //crearea socket-ului
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_port = htons (PORT);
  
  //conectarea la server
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }


  //dupa conectare se trimite serverului pid-ul procesului curent
  int pid_number = getpid();
  char * pid_to_send = malloc(100);
  pid_to_send[0] = 0;
  sprintf(pid_to_send,"%d",pid_number);
  pid_to_send[strlen(pid_to_send)] = 0;
  
  if (write (sd,pid_to_send,strlen(pid_to_send)+1) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }


  char username[100];
  bzero(&username, sizeof(char));
  

  printf("\nBine ati venit la QuizzGame! \n[Reguli]: Pentru a incepe jocul alegeti un username care nu contine spatii. Pentru a abandona jocul scrieti quit sau opriti programul. Username-ul vostru nu poate fi cuvantul rezervat quit!\n\n");

  printf ("[client]Introduceti username: ");
  fflush (stdout);

  char buf[100]; 
  scanf("%79s[^\n]", buf); 
  if (sscanf(buf, "%s", username) == 1)

  //verificam daca utilizatorul vrea sa iasa din joc
  if(strlen(username) == strlen("quit"))
  {
    if(strstr(username,"quit"))
      return 0;
  }
  printf("Username-ul dat este: %s\n", username);

  
  //trimitem la server username-ul introdus de utilizator
  if (write (sd,username,strlen(username)+1) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }

    char * rasp=malloc(100);

  //citim raspunsul de la server
  if (read (sd, rasp,100) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  printf ("[client]: %s\n", rasp);


  //cat timp username-ul introdus de utilizator nu este disponibil, se cere jucatorului un alt username
  while(strstr(rasp,"Username already taken"))
  {
    printf("[client]: Incercati alt username: ");
    fflush(stdout);

    bzero(&username, sizeof(char));
    scanf("%79s", buf); 
    sscanf(buf, "%s", username);
    printf("Username-ul dat este: %s_%ld\n", username, strlen(username));

    //verificam daca utilizatorul vrea iasa din joc
    if(strlen(username) == strlen("quit"))
  {
    if(strstr(username,"quit"))
      return 0;
  }

    // trimiterea mesajului la server 
    if (write (sd,username,strlen(username)+1) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
      }

    if (read (sd, rasp,100) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  printf ("[client]: %s\n", rasp);

  }

  //din acest punct jucatorul va primi intrebarile de la server
  printf("[client]: Esti gata sa incepi jocul!\n");

  int nr_intrebari;

  if(read(sd, &nr_intrebari,sizeof(int))<0)
  {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
  }

  printf("Vei avea de raspuns la %d intrebari!\n\n",nr_intrebari);


  for(int i = 1; i <= nr_intrebari; i++ )
  {
    //citirea intrebarii si a raspunsurilor acesteia si afisarea acestora pentru utilizator
    char *intrebare_raspunsuri=malloc(1000);
    if( read(sd,intrebare_raspunsuri,1000) < 0 )
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }

    printf("[Intrebare]: %s\n",intrebare_raspunsuri);
    free(intrebare_raspunsuri);
    
    printf("Alege raspunsul: ");    
    pthread_t threads[2];
    pthread_t thread_1;
    pthread_t thread_2;
    
       thread_1 = pthread_create(&threads[0], NULL, Reading, NULL);
       thread_2 = pthread_create(&threads[1], NULL, Timer, NULL);
       if (thread_1)
       {
          printf("ERROR; return code from pthread_create() is \n");
          exit(-1);
       }

       if (thread_2)
       {
          printf("ERROR; return code from pthread_create() is \n");
          exit(-1);
       }
    

    while(exited == 0)
    {
        if(check_if_answered == 1)
        {
            pthread_cancel(threads[1]);
            break;
        }
    }


    if(check_if_answered == 1)
    {
        printf("S-a primit mesajul: %s\n\n",raspuns);
    }
    else
    {
        pthread_cancel(threads[0]);
        printf("Nu s-a dat niciun raspuns!\n\n");
        strcpy(raspuns,"no_answer");
    }

    if(write(sd,raspuns,strlen(raspuns)+1)<=0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }

    if(strlen("quit") == strlen(raspuns))
    {
      if(strstr("quit",raspuns))
        return 0;
    }

    exited = 0;
    check_if_answered = 0;

  }
  
  fflush(stdout);

  int punctaj_obtinut = 0;

    if( read(sd,&punctaj_obtinut,sizeof(punctaj_obtinut)) < 0 )
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }

  fflush(stdout);
  printf("Ai obtinut %d puncte!\n",punctaj_obtinut);

  //se asteapta ca toti utilizatorii inregistrati in runda curenta sa termine intrebarile si abia apoi se afiseaza castigatorul
  printf("Asteptam rezultatele jocului!\n");

    
  char * info_castigator = malloc(100);
  info_castigator[0]=0;
  if( read(sd,info_castigator,100) < 0 )
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }

  printf("%s\n",info_castigator);
  

  close (sd);
  pthread_exit(NULL);

  return 0;
}
