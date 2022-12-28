/* cliTCPIt.c - Exemplu de client TCP
   Trimite un nume la server; primeste de la server "Hello nume".
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

int port = 6969;              // portul de conecatre la server
extern int errno;             // codul de eroare returnat de anumite apeluri
void info()
{
  int sd;			                // descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char buf[1000];
  char reply[1000];
 

  strcpy(buf,"info\n");
    
  //printf("[client] Am citit %s\n",buf);
    
    /* cream socketul */

  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */

  server.sin_family = AF_INET;                      // familia socket-ului 
  server.sin_addr.s_addr = inet_addr("127.0.0.1");  // adresa IP a serverului
  server.sin_port = htons (port);                   // portul de conectare
  
  /* ne conectam la server */

  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }


    /* trimiterea mesajului la server */
    if (write (sd,buf,sizeof(buf)) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
      }

    /* citirea raspunsului dat de server 
      (apel blocant pina cind serverul raspunde) */
    if (read (sd, reply,sizeof(reply)) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
      }
    /* afisam mesajul primit */
    printf ("%s\n", reply);

  /* inchidem conexiunea, am terminat */
  close (sd);
}

int main (int argc, char *argv[])
{
  info();                     // afiseaza informatii despre trenuri fara input de la utilizator
  while (1){
    int sd;			                // descriptorul de socket
    struct sockaddr_in server;	// structura folosita pentru conectare 
    char buf[1000];
    char reply[1000];
  

    /* citirea mesajului */
    
      printf (">");
      fflush (stdout);
      fgets(buf, 1000, stdin);
      //printf("\n");
     //printf("[client] Am citit %s\n",buf);

      /* cream socketul */

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
      {
        perror ("Eroare la socket().\n");
        return 0;
      }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */

    server.sin_family = AF_INET;                      // familia socket-ului 
    server.sin_addr.s_addr = inet_addr("127.0.0.1");  // adresa IP a serverului
    server.sin_port = htons (port);                   // portul de conectare
    
    /* ne conectam la server */

    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
      {
        perror ("[client]Eroare la connect().\n");
        return errno;
      }


      /* trimiterea mesajului la server */
      if (write (sd,buf,sizeof(buf)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }

      /* citirea raspunsului dat de server 
        (apel blocant pina cind serverul raspunde) */
      if (read (sd, reply,sizeof(reply)) < 0)
        {
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
      /* afisam mesajul primit */
      printf ("%s\n", reply);

    /* inchidem conexiunea, am terminat */
    close (sd);
  }
}
