#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdint.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <strings.h>

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;
struct stat sts;

struct player
{
  char* username;
  int punctaj;
};

struct player *vector;
int number_of_players;
int capacity_players = 100;
struct player castigator;
int all_finished;
int vector_intrebari[10];

pthread_mutex_t mutex_castigator;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;


//definitii functii utilizate
static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
int check_username(void *, char *);
int game(void*,int, char*);
int username_unic(char*);
int check_if_player_exited(char * pid);
char* get_player_pid(void *);
void initializare_vector_intrebari();

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;

  number_of_players=0;
  vector = (struct player*)malloc(capacity_players);
  all_finished = 0;
  castigator.username = malloc(100);
  castigator.punctaj = 0;
  pthread_mutex_init(&mutex_castigator, NULL);
  initializare_vector_intrebari();
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		

    char * pid = malloc(100);
    pid[0] = 0;
    strcpy(pid, get_player_pid((struct thData*)arg));

    //verificare primire pid

    printf("Pidului playerului este: %s\n",pid);

		int id_player = check_username((struct thData*)arg, pid);

    if(id_player != -1)
    {
      game((struct thData*)arg, id_player, pid);
    }

		//inchidem conexiunea pentru acest jucator

		close ((intptr_t) arg);

		return(NULL);	
  		
};


int check_username(void *arg, char * pid)
{
    int i=0;
    struct thData tdL; 
    tdL= *((struct thData*)arg);


    int valid = check_if_player_exited(pid);
    printf("Am verificat daca pidul este valid: %d\n", valid);

    if(valid == 0)
    {
      return -1;
    }
    
    char *buffer=malloc(100);
	  if (read (tdL.cl, buffer,100) <= 0)
			{
			  printf("[Thread %d]\n",tdL.idThread);
			  perror ("Eroare la read() de la client.\n");
			
			}

    
    if( check_if_player_exited(pid) == 0)
    {
      return -1;
    }
    if(strlen(buffer) == strlen("quit"))
    {
      if(strstr(buffer,"quit"))
        return -1;
    }
	
	printf ("[Thread %d]Mesajul a fost receptionat...%s_%ld\n",tdL.idThread, buffer, strlen(buffer));

  while(username_unic(buffer)==0)
  {
    /*pregatim mesajul de raspuns */
	  char *raspuns=malloc(100);
    raspuns[0]=0;
    strcat(raspuns,"Username already taken!");
    raspuns[strlen(raspuns)]=0;

	  printf("[Thread %d]Trimitem mesajul inapoi...%s\n",tdL.idThread, raspuns);
		      
     if(check_if_player_exited(pid) == 0)
    {
      return -1;
    }     
		      
		/* returnam mesajul clientului */
    if (write (tdL.cl, raspuns, strlen(raspuns)+1) <= 0)
      {
      printf("[Thread %d] ",tdL.idThread);
      perror ("[Thread]Eroare la write() catre client.\n");
      }
    else
      printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	

      free(raspuns);

      if(check_if_player_exited(pid) == 0)
      {
        return -1;
      }

      if (read (tdL.cl, buffer,100) <= 0)
			{
			  printf("[Thread %d]\n",tdL.idThread);
			  perror ("Eroare la read() de la client.\n");
			
			}

      if(check_if_player_exited(pid) == 0)
    {
      return -1;
    }     

    if(strlen(buffer) == strlen("quit"))
    {
      if(strstr(buffer,"quit"))
        return -1;
    }
	
	    printf ("[Thread %d]Mesajul a fost receptionat...%s_%ld\n",tdL.idThread, buffer,strlen(buffer));

  }

  if(number_of_players + 1 > capacity_players)
  {
      capacity_players = capacity_players + 100;
      struct player * tmp = realloc(vector,capacity_players);
      if (tmp == NULL) 
      {
          return 0;
      }
      vector = tmp;
  }


  pthread_mutex_lock (&mutex_castigator);

  vector[number_of_players].username=malloc(100);
  strcpy(vector[number_of_players].username,buffer);
  vector[number_of_players].punctaj=0;
  number_of_players++;
  pthread_mutex_unlock (&mutex_castigator);

  for(int i=0; i<number_of_players; i++)
  {
    printf("player:%d, username:%s",i,vector[i].username);
  }
		      
		/*pregatim mesajul de raspuns */
	  char *raspuns=malloc(100);
    raspuns[0]=0;
    strcat(raspuns,"V-ati inregistrat cu succes, ");
    strcat(raspuns,buffer);
    raspuns[strlen(raspuns)]=0;

	printf("[Thread %d]Trimitem mesajul inapoi...%s\n",tdL.idThread, raspuns);     
		      
		      /* returnam mesajul clientului */

    if(check_if_player_exited(pid) == 0)
    {
      return -1;
    }

	 if (write (tdL.cl, raspuns, strlen(raspuns)+1) <= 0)
		{
		 printf("[Thread %d] ",tdL.idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
	else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	

    if(check_if_player_exited(pid) == 0)
    {
      return -1;
    }

    
    free(raspuns);
    
  return (number_of_players-1);
}

//functie pentru a verifica daca a fost luat deja username-ul introdus
int username_unic(char *arg)
{
  for(int i=0; i<number_of_players; i++)
  {
    if(strlen(vector[i].username)==strlen(arg))
      if(strstr(vector[i].username,arg))
        return 0;
  }
  return 1;
}


int game(void *arg, int id_player, char * pid)
{
    int i=0;
    struct thData tdL; 
    tdL= *((struct thData*)arg);

    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;
    
    int rc = sqlite3_open("database.db", &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        //return 1;
    }
    
    char *sql = "SELECT id, text_intrebare, raspuns_1, raspuns_2, raspuns_3, raspuns_4, raspuns_corect FROM intrebari WHERE id = ?";
        
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    
    if (rc == SQLITE_OK) {
        
        //sqlite3_bind_int(res, 1, 1);
    } else {
        
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }


    int nr_intrebari=3;

    //trimitem inainte numarul de intrebari ca sa stim cate citiri sa facem in client
    if(write(tdL.cl,&nr_intrebari,sizeof(int))<=0)
    {
      printf("[Thread %d] ",tdL.idThread);
		  perror ("[Thread]Eroare la write() catre client.\n");
    }

    printf("[Thread %d]: Mesajul cu numarul de intrebari a fost trasnmis cu succes!\n",tdL.idThread);


    for(int i=1; i<=9; i++)
    {
        if(vector_intrebari[i] == 1)
      {

        rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

        if (rc == SQLITE_OK) 
        {
            sqlite3_bind_int(res, 1, i);
        } 
        else 
        {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        }

        int step = sqlite3_step(res);

        
        if (step == SQLITE_ROW) {

            char * intrebare=malloc(100); intrebare[0]=0;
            char * raspuns_1=malloc(100); raspuns_1[0]=0;
            char * raspuns_2=malloc(100); raspuns_2[0]=0;
            char * raspuns_3=malloc(100); raspuns_3[0]=0;
            char * raspuns_4=malloc(100); raspuns_4[0]=0;
            char * raspuns_corect=malloc(100); raspuns_corect[0]=0;
            char * sir_de_trimis=malloc(1000); sir_de_trimis[0]=0;
            
            strcpy(intrebare,sqlite3_column_text(res, 1)); //punem in variabila intrebare continutul coloanei 1 de pe randul indicat de id
            strcpy(raspuns_1,sqlite3_column_text(res, 2));
            strcpy(raspuns_2,sqlite3_column_text(res, 3));
            strcpy(raspuns_3,sqlite3_column_text(res, 4));
            strcpy(raspuns_4,sqlite3_column_text(res, 5));
            strcpy(raspuns_corect,sqlite3_column_text(res, 6));
            
            char *question_number=malloc(10);
            sprintf(question_number,"%d. ",i);

            //formatam intrebarea si raspunsurile
            //strcat(sir_de_trimis,question_number);
            strcat(sir_de_trimis, intrebare);
            strcat(sir_de_trimis,"\n");
            strcat(sir_de_trimis,raspuns_1);
            strcat(sir_de_trimis,"\n");
            strcat(sir_de_trimis,raspuns_2);
            strcat(sir_de_trimis,"\n");
            strcat(sir_de_trimis,raspuns_3);
            strcat(sir_de_trimis,"\n");
            strcat(sir_de_trimis, raspuns_4);
            strcat(sir_de_trimis,"\n");

            sir_de_trimis[strlen(sir_de_trimis)]=0;

            if(check_if_player_exited(pid) == 0)
            {
              for(int index = id_player; i < number_of_players - 1; i++)
              {
                  vector[index] = vector[index+1];
              }
              number_of_players--;
              free(vector[number_of_players].username);
              vector[number_of_players].punctaj = 0;

              return -1;
            }


            if(write(tdL.cl,sir_de_trimis,strlen(sir_de_trimis)+1)<=0)
            {
              printf("[Thread %d] ",tdL.idThread);
              perror ("[Thread]Eroare la write() catre client.\n");
            }

            if(check_if_player_exited(pid) == 0)
            {
              for(int index = id_player; i < number_of_players - 1; i++)
              {
                  vector[index] = vector[index+1];
              }
              number_of_players--;
              free(vector[number_of_players].username);
              vector[number_of_players].punctaj = 0;

              return -1;
            }

            printf("Thread %d]: Am transmis cu succes intrebarile si raspunsurile!\n",tdL.idThread);

            free(intrebare); free(raspuns_1); free(raspuns_2); free(sir_de_trimis);

            char * player_answer=malloc(100);

            //citim raspunsul la intrebare al jucatorului

            if (read (tdL.cl, player_answer,100) <= 0)
            {
              printf("[Thread %d]\n",tdL.idThread);
              perror ("Eroare la read() de la client.\n");
            }

            if(check_if_player_exited(pid) == 0)
            {
              for(int index = id_player; i < number_of_players - 1; i++)
              {
                  vector[index] = vector[index+1];
              }
              number_of_players--;
              free(vector[number_of_players].username);
              vector[number_of_players].punctaj = 0;

              return -1;
            }

            printf("Raspunsul jucatorului este: %s\n",player_answer);

            if(strlen(player_answer) == strlen(raspuns_corect))
            {
              if(strstr(player_answer,raspuns_corect))
              vector[id_player].punctaj+=5;
            }
            if(strlen(player_answer) == strlen("quit"))
            {
              if(strstr("quit",player_answer))
              {
              for(int index = id_player; i < number_of_players - 1; i++)
              {
                  vector[index] = vector[index+1];
              }
              number_of_players--;

              return -1;
              }
            }

            free(player_answer);
          }
      } 

    }
    char * punctaj_player=malloc(100);
    punctaj_player[0]=0;
    sprintf(punctaj_player,"Ai obtinut %d puncte!",vector[id_player].punctaj);
    punctaj_player[strlen(punctaj_player)]=0;

    if(write(tdL.cl,&vector[id_player].punctaj,sizeof(int))<=0)
    {
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread]Eroare la write() catre client.\n");
    }

    if(check_if_player_exited(pid) == 0)
        {
          for(int index = id_player; i < number_of_players - 1; i++)
          {
              vector[index] = vector[index+1];
          }
          number_of_players--;
          free(vector[number_of_players].username);
          vector[number_of_players].punctaj = 0;

          return -1;
        }

    
    pthread_mutex_lock (&mutex_castigator);
    if(castigator.punctaj<vector[id_player].punctaj)
    {
        strcpy(castigator.username, vector[id_player].username);
        castigator.punctaj = vector[id_player].punctaj; 
    }
    


    all_finished = all_finished + 1;
    pthread_mutex_unlock (&mutex_castigator);

    printf("\nNUMARUL JUCATORILOR ESTE: %d\n", number_of_players);
    printf("Variabila finished are valoarea: %d\n",all_finished);

    
    while(all_finished != number_of_players)
    {
      //wait for all players to finish the game so we can announce the winner
    }

    char * announce_winner = malloc(100);
    announce_winner[0] = 0;
    
    if(vector[id_player].punctaj == castigator.punctaj && castigator.punctaj !=0 )
    {
      sprintf(announce_winner,"Ai castigat!");
    }
    else
    {
      if(castigator.punctaj == 0)
        {
          sprintf(announce_winner,"Nu exista niciun castigator!");
        }
      else
      {
        sprintf(announce_winner,"Castigatorul este %s", castigator.username);
      }
    }


    if(write(tdL.cl,announce_winner,strlen(announce_winner)+1)<=0)
    {
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread]Eroare la write() catre client.\n");
    }


    sqlite3_finalize(res);
    sqlite3_close(db);

    //il scoatem pe jucator din vectorul de jucatori
    pthread_mutex_lock (&mutex_castigator);
    for(int index = id_player; i < number_of_players - 1; i++)
          {
              vector[index] = vector[index+1];
          }
    number_of_players--;
    bzero(&vector[number_of_players].username,sizeof(char*));
    vector[number_of_players].punctaj = 0;
    all_finished--;

    if(number_of_players == 0)
    {
      castigator.punctaj = 0;
      initializare_vector_intrebari();
    }
    pthread_mutex_unlock (&mutex_castigator);

    return 0;

}


int check_if_player_exited(char * pid)
{

    char * check_pid = malloc(100);
    check_pid[0] = 0;
    strcpy(check_pid,"/proc/");
    strcat(check_pid,pid);
    check_pid[strlen(check_pid)] = 0;

    if (stat(check_pid, &sts) == -1 && errno == ENOENT) 
    {
      return 0; //procesul nu exista
    }
    else
   {
      return 1; //procesul exista
    }

}


char* get_player_pid(void * arg)
{
    //citire a pidului procesului
    struct thData tdL; 
    tdL= *((struct thData*)arg);
    

    char *buffer=malloc(100);
	  if (read (tdL.cl, buffer,100) <= 0)
			{
			  printf("[Thread %d]\n",tdL.idThread);
			  perror ("Eroare la read() de la client.\n");
			
			}
	
	  printf ("[Thread %d]Mesajul a fost receptionat...%s\n",tdL.idThread, buffer);

    return buffer;


}


void initializare_vector_intrebari()
{
  for(int i=0; i<10; i++)
  {
    vector_intrebari[i] = 0;
  }

  int a,b,c;
  srand(time(NULL));
  a = rand() %9 +1;
    
  srand(time(NULL));
  b = rand() %9 +1;
  while(a==b)
  {
      srand(time(NULL));
      b = rand() %9 +1;
  }

  srand(time(NULL));
  c = rand() %9 +1;

  while(a==c || b==c)
  {
       srand(time(NULL));
      c = rand() %9 +1;
  }

  vector_intrebari[a] = 1;
  vector_intrebari[b] = 1;
  vector_intrebari[c] = 1;

}







