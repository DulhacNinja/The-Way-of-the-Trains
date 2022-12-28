/* servTCPPreTh.c - Exemplu de server TCP concurent care deserveste clientii
   printr-un mecanism de prethread-ing; cu blocarea mutex de protectie a lui accept();
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.


   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

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
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <string.h>

int id = 7; // id for trains
char *itoa(int number)
{
  char n;
  char buf[101];
  int i = 0;
  do
  {
    n = (number % 10) + '0';
    buf[i] = n;
    i = i + 1;
    number = number / 10;
  } while (number);
  buf[i] = '\0';
  // printf("buffer: %s \n",buf);
  char rev[101];
  for (int j = 0; j < i; j++)
    rev[j] = buf[i - j - 1];
  rev[i] = '\0';
  char *ptr;
  ptr = rev;
  // printf("rev: %s \n",ptr);
  return ptr;
}
int convert_time_minutes(char *time)
{
  int hours, minutes;
  if (time[1] != ':')
  {
    hours = (time[0] - '0') * 10 + (time[1] - '0');
    minutes = (time[3] - '0') * 10 + (time[4] - '0');
  }
  else
  {
    hours = (time[0] - '0');
    minutes = (time[2] - '0') * 10 + (time[3] - '0');
  }
  int result = hours * 60 + minutes;
  // printf("%d hours, %d minutes",hours,minutes);
  // printf("time: %s -> in mins: %d\n", time, result);
  return result;
}
char *convert_minutes_time(int minutes)
{
  int hours = (minutes / 60) % 24;
  int mins = minutes % 60;
  char result[10];
  result[0] = (char)((hours / 10) + '0');
  result[1] = (char)((hours % 10) + '0');
  result[2] = ':';
  result[3] = (char)((mins / 10) + '0');
  result[4] = (char)((mins % 10) + '0');
  result[5] = '\0';
  printf("result = %s\n", result);
  char *ptr;
  ptr = result;
  return ptr;
}
/* imi da print la tot xml-ul */
char *print_train(xmlNode *train)
{
  // printf("ID : %s\n", xmlGetProp(train,"id"));
  char buf[100];
  buf[0] = '\0';
  strcat(buf, "ID : ");
  strcat(buf, xmlGetProp(train, "id"));
  strcat(buf, " ");

  xmlNode *city = train->children;
  // printf("%s -> ", xmlNodeGetContent(city)); // departure city last train
  strcat(buf, xmlNodeGetContent(city));
  strcat(buf, " -> ");
  city = city->next;
  // printf("%s\n", xmlNodeGetContent(city)); // arival city last train
  strcat(buf, xmlNodeGetContent(city));
  strcat(buf, " ");
  xmlNode *departure_time = city->next;
  // printf("Departs at %s \n", xmlNodeGetContent(departure_time));                    // departure time
  strcat(buf, "Departs at ");
  strcat(buf, xmlNodeGetContent(departure_time));
  strcat(buf, " ");
  if (strcmp(xmlGetProp(departure_time, "delay"), "0") != 0){
    strcat(buf, "DELAYED BY ");
  strcat(buf, xmlGetProp(departure_time, "delay"));
  strcat(buf, " ");
  }
  xmlNode *arrival_time = departure_time->next;
  // printf("Arrives at %s \n", xmlNodeGetContent(arrival_time));                    // arrival time
  strcat(buf, "Arrives at ");
  strcat(buf, xmlNodeGetContent(arrival_time));
  if (strcmp(xmlGetProp(arrival_time, "delay"), "0") != 0){
    strcat(buf, "DELAYED BY ");
  strcat(buf, xmlGetProp(arrival_time, "delay"));
  strcat(buf, " ");
  }
  strcat(buf, "\n");
  char *ptr = buf;
  return ptr;
}
char *print_all(xmlNode *root)
{
  xmlNode *train = root->children;
  char buf[1000];
  buf[0] = '\0';
  while (train)
  {
    strcat(buf, print_train(train));
    train = train->next;
  }
  char *ptr = buf;
  return ptr;
}

char *trains_departure_hour(xmlNode *root, int hour1, int hour2) // hour e specificat in minute, root e list, rootul la lista
{
  xmlNode *train = root->children;
  char buf[1000];
  buf[0] = '\0';
  while (train)
  {
    // we get the time of departure
    xmlNode *aux = train;
    xmlNode *departure_time = aux->children->next->next;
    int time = convert_time_minutes(xmlNodeGetContent(departure_time));

    // printf("%d : iar timpul citit este %s \n",time, xmlNodeGetContent(departure_time));

    if (hour1 <= time && time <= hour2)
      strcat(buf, print_train(train));
    train = train->next;
  }
  char *ptr = buf;
  return ptr;
}
char *trains_arrival_hour(xmlNode *root, int hour1, int hour2) // hour e specificat in minute, root e list, rootul la lista
{
  xmlNode *train = root->children;
  char buf[1000];
  buf[0] = '\0';
  while (train)
  {
    // we get the time of arrival
    xmlNode *aux = train;
    xmlNode *arrival_time = aux->children->next->next->next;
    int time = convert_time_minutes(xmlNodeGetContent(arrival_time));
    // printf("%d : iar timpul citit este %s \n",time, xmlNodeGetContent(arrival_time));

    if (hour1 <= time && time <= hour2)
      strcat(buf, print_train(train));
    train = train->next;
  }
  char *ptr = buf;
  return ptr;
}
void new_train(xmlNode *root, char *id_char, char *departure_city, char *arrival_city, char *departure_time, char *arrival_time)
{
  xmlNode *new_train = xmlNewChild(root, NULL, BAD_CAST "train", NULL);
  // printf("ID = %s\n", id_char);
  xmlNewProp(new_train, BAD_CAST "id", BAD_CAST id_char); // we'll do smth about the id
  // xmlNewProp(new_train, BAD_CAST "id", BAD_CAST "7");
  xmlNewChild(new_train, NULL, BAD_CAST "departs_from", departure_city);
  xmlNewChild(new_train, NULL, BAD_CAST "arrives_in", arrival_city);
  xmlNode *departure = xmlNewChild(new_train, NULL, BAD_CAST "departs_at", departure_time);
  xmlNewProp(departure, BAD_CAST "delay", BAD_CAST "0");
  xmlNode *arrival = xmlNewChild(new_train, NULL, BAD_CAST "arrives_at", arrival_time);
  xmlNewProp(arrival, BAD_CAST "delay", BAD_CAST "0");

  return;
}
void delete_train(xmlNode *root, char *id_char)
{
  xmlNode *train = root->children;
  xmlNode *future;
  while (train)
  {
    char *train_id = xmlGetProp(train, "id");
    future = train->next;
    if (strcmp(train_id, id_char) == 0)
    {
      xmlUnlinkNode(train);
      xmlFreeNode(train);
    }
    train = future;
  }
}
void delay_departure_train(xmlNode *root, char *id_char, char *delay)
{
  xmlNode *train = root->children;
  xmlNode *future;
  while (train)
  {
    char *train_id = xmlGetProp(train, "id");
    future = train->next;
    if (strcmp(train_id, id_char) == 0)
    {
      xmlNode *departure_time = train->children->next->next;
      xmlSetProp(departure_time, "delay", delay);
      printf("OK2\n");
      int current_time = convert_time_minutes(xmlNodeGetContent(departure_time));
      printf("%d\n",current_time);
      printf("%s\n",delay);
      printf("%d\n",atoi(delay));
      int new_time = current_time + atoi(delay);
      char* ptr = convert_minutes_time(new_time);
      printf("%s\n",ptr);
      printf("%d\n",strlen(ptr));
      xmlNodeSetContent(departure_time, BAD_CAST ptr);
    }
    train = future;
  }
  return;
}
void delay_arrival_train(xmlNode *root, char *id_char, char *delay)
{
  xmlNode *train = root->children;
  xmlNode *future;
  while (train)
  {
    char *train_id = xmlGetProp(train, "id");
    future = train->next;
    if (strcmp(train_id, id_char) == 0)
    {
      xmlNode *arrival_time = train->children->next->next->next;
      xmlSetProp(arrival_time, "delay", delay);
      int current_time = convert_time_minutes(xmlNodeGetContent(arrival_time));
      printf("%s",convert_minutes_time(current_time + atoi(delay)));
     // xmlNodeSetContent(arrival_time, BAD_CAST convert_minutes_time(current_time + atoi(delay)));
    }
    train = future;
  }
  return;
}
/* portul folosit */
#define PORT 6969

/* codul de eroare returnat de anumite apeluri */
extern int errno;

static void *treat(void *); // functia executata de fiecare thread ce realizeaza comunicarea cu clientii */

typedef struct
{
  pthread_t idThread; // id-ul thread-ului
  int thCount;        // nr de conexiuni servite
} Thread;

Thread *threadsPool;                               // un array de structuri Thread
int sd;                                            // descriptorul de socket de ascultare
int nthreads = 10;                                 // numarul de threaduri
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER; // variabila mutex ce va fi partajata de threaduri

void raspunde(int cl, int idThread);

int main(int argc, char *argv[])
{
  struct sockaddr_in server; // structura folosita de server
  void threadCreate(int);    //

  threadsPool = calloc(sizeof(Thread), nthreads);

  /* crearea unui socket */

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }
  /* utilizarea optiunii SO_REUSEADDR */

  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  /* pregatirea structurilor de date */
  bzero(&server, sizeof(server));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;
  /* acceptam orice adresa */
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  /* utilizam un port utilizator */
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }

  printf("Nr threaduri %d \n", nthreads);
  fflush(stdout);
  int i;
  for (i = 0; i < nthreads; i++)
    threadCreate(i);

  /* servim in mod concurent clientii...folosind thread-uri */
  for (;;)
  {
    printf("[server]Asteptam la portul %d...\n", PORT);
    pause();
  }
};

void threadCreate(int i)
{
  void *treat(void *);

  pthread_create(&threadsPool[i].idThread, NULL, &treat, (void *)i);
  return; /* threadul principal returneaza */
}

void *treat(void *arg)
{
  int client;

  struct sockaddr_in from;
  bzero(&from, sizeof(from));
  printf("[thread]- %d - pornit...\n", (int)arg);
  fflush(stdout);

  for (;;)
  {
    int length = sizeof(from);
    pthread_mutex_lock(&mlock);
    // printf("Thread %d trezit\n",(int)arg);
    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[thread]Eroare la accept().\n");
    }

    pthread_mutex_unlock(&mlock);
    threadsPool[(int)arg].thCount++;

    raspunde(client, (int)arg); // procesarea cererii
    /* am terminat cu acest client, inchidem conexiunea */
    close(client);
  }
}

// this is where the functionality happens
void raspunde(int cl, int idThread)
{
  char buf[1000]; // mesajul primit de trimis la client
  char reply[1000];
  if (read(cl, buf, sizeof(buf)) <= 0)
  {
    printf("[Thread %d]\n", idThread);
    perror("Eroare la read() de la client.\n");
  }

  printf("[Thread %d]Mesajul a fost receptionat...%s\n", idThread, buf);

  /*pregatim mesajul de raspuns */
  // strcpy(reply, buf);

  xmlDoc *doc = NULL;
  xmlNode *root = NULL;

  doc = xmlReadFile("trains.xml", "UTF-8", 0);
  root = xmlDocGetRootElement(doc);

  if (strcmp(buf, "info\n") == 0)
  {
    strcpy(reply, print_all(root));
  }
  if( strcmp(buf, "help\n") == 0)
  {
    strcpy(reply, "info -> all curent trains\nadd <ID> <CITY_DEPARTURE> <CITY_ARRIVAL> <TIME_DEPARTURE> <TIME_ARRIVAL> -> adds a train\n");
    strcat(reply, "remove <ID> -> removes a train\ndepartures <TIME_1> <TIME_2> -> returns all trains with departures between TIME_1 AND TIME_2\n");
    strcat(reply, "arrivals <TIME_1> <TIME_2> -> same as departures, but with arrivals\ndelay (departure | arrival) <ID> <MINUTES> -> add a delay to a train, doesnt work");
  }
  if (buf[0] == 'd' &&
      buf[1] == 'e' &&
      buf[2] == 'p' &&
      buf[3] == 'a' &&
      buf[4] == 'r' &&
      buf[5] == 't' &&
      buf[6] == 'u' &&
      buf[7] == 'r' &&
      buf[8] == 'e' &&
      buf[9] == 's' &&
      buf[10] == ' ')
  {
    int index = 11;
    while (buf[index] == ' ')
      index = index + 1;
    char time_1[5];
    int time_index_1 = 0;
    while (buf[index] != ' ')
    {
      time_1[time_index_1] = buf[index];
      time_index_1 = time_index_1 + 1;
      index = index + 1;
    }
    time_1[time_index_1] = '\0';
    while (buf[index] == ' ')
      index = index + 1;
    char time_2[5];
    int time_index_2 = 0;
    while (buf[index] != '\0' && buf[index] != ' ' && buf[index] != '\n')
    {
      time_2[time_index_2] = buf[index];
      time_index_2 = time_index_2 + 1;
      index = index + 1;
    }
    time_2[time_index_2] = '\0';
    strcpy(reply, (trains_departure_hour(root, convert_time_minutes(time_1), convert_time_minutes(time_2))));
  }
  if (buf[0] == 'a' &&
      buf[1] == 'r' &&
      buf[2] == 'r' &&
      buf[3] == 'i' &&
      buf[4] == 'v' &&
      buf[5] == 'a' &&
      buf[6] == 'l' &&
      buf[7] == 's' &&
      buf[8] == ' ')
  {
    int index = 9;
    while (buf[index] == ' ')
      index = index + 1;
    char time_1[5];
    int time_index_1 = 0;
    while (buf[index] != ' ')
    {
      time_1[time_index_1] = buf[index];
      time_index_1 = time_index_1 + 1;
      index = index + 1;
    }
    time_1[time_index_1] = '\0';
    while (buf[index] == ' ')
      index = index + 1;
    char time_2[5];
    int time_index_2 = 0;
    while (buf[index] != '\0' && buf[index] != ' ' && buf[index] != '\n')
    {
      time_2[time_index_2] = buf[index];
      time_index_2 = time_index_2 + 1;
      index = index + 1;
    }
    time_2[time_index_2] = '\0';
    strcpy(reply, (trains_arrival_hour(root, convert_time_minutes(time_1), convert_time_minutes(time_2))));
  }
  if (buf[0] == 'a' &&
      buf[1] == 'd' &&
      buf[2] == 'd' &&
      buf[3] == ' ')
  {
    int index = 4;
    while (buf[index] == ' ')
      index = index + 1;
    char id_char[100];
    int id_index = 0;
    while (buf[index] != ' ')
    {
      id_char[id_index] = buf[index];
      id_index = id_index + 1;
      index = index + 1;
    }
    id_char[id_index] = '\0';

    while (buf[index] == ' ')
      index = index + 1;
    char city_1[100];
    int city_index_1 = 0;
    while (buf[index] != ' ')
    {
      city_1[city_index_1] = buf[index];
      city_index_1 = city_index_1 + 1;
      index = index + 1;
    }
    city_1[city_index_1] = '\0';

    while (buf[index] == ' ')
      index = index + 1;
    char city_2[100];
    int city_index_2 = 0;
    while (buf[index] != ' ')
    {
      city_2[city_index_2] = buf[index];
      city_index_2 = city_index_2 + 1;
      index = index + 1;
    }
    city_2[city_index_2] = '\0';

    while (buf[index] == ' ')
      index = index + 1;
    char time_1[100];
    int time_index_1 = 0;
    while (buf[index] != ' ')
    {
      time_1[time_index_1] = buf[index];
      time_index_1 = time_index_1 + 1;
      index = index + 1;
    }
    time_1[time_index_1] = '\0';

    while (buf[index] == ' ')
      index = index + 1;
    char time_2[100];
    int time_index_2 = 0;
    while (buf[index] != '\0' && buf[index] != ' ' && buf[index] != '\n')
    {
      time_2[time_index_2] = buf[index];
      time_index_2 = time_index_2 + 1;
      index = index + 1;
    }
    time_2[time_index_2] = '\0';

    new_train(root, id_char, city_1, city_2, time_1, time_2);
    strcpy(reply, "Train added!\n");
    // printf("%s %s %s %s",city_1, city_2, time_1, time_2);
  }
  if (buf[0] == 'r' &&
      buf[1] == 'e' &&
      buf[2] == 'm' &&
      buf[3] == 'o' &&
      buf[4] == 'v' &&
      buf[5] == 'e' &&
      buf[6] == ' ')
  {
    int index = 7;
    while (buf[index] == ' ')
      index = index + 1;
    char id_char[100];
    int id_char_index = 0;
    while (buf[index] != '\0' && buf[index] != ' ' && buf[index] != '\n')
    {
      id_char[id_char_index] = buf[index];
      id_char_index = id_char_index + 1;
      index = index + 1;
    }
    id_char[id_char_index] = '\0';

    delete_train(root, id_char);
    strcpy(reply, "Train removed!\n");
  }
  if (buf[0] == 'd' &&
      buf[1] == 'e' &&
      buf[2] == 'l' &&
      buf[3] == 'a' &&
      buf[4] == 'y' &&
      buf[5] == ' ')
  {
    if (
        buf[6] == 'a' &&
        buf[7] == 'r' &&
        buf[8] == 'r' &&
        buf[9] == 'i' &&
        buf[10] == 'v' &&
        buf[11] == 'a' &&
        buf[12] == 'l' &&
        buf[13] == ' ')
    {
      int index = 14;
      while (buf[index] == ' ')
        index = index + 1;
      char id_char[100];
      int id_char_index = 0;
      while (buf[index] != '\0' && buf[index] != ' ' && buf[index] != '\n')
      {
        id_char[id_char_index] = buf[index];
        id_char_index = id_char_index + 1;
        index = index + 1;
      }
      id_char[id_char_index] = '\0';
      while (buf[index] == ' ')
        index = index + 1;
      char delay[100];
      int delay_index = 0;
      while (buf[index] != '\0' && buf[index] != ' ' && buf[index] != '\n')
      {
        delay[delay_index] = buf[index];
        delay_index = delay_index + 1;
        index = index + 1;
      }
      delay[delay_index] = '\0';
      delay_arrival_train(root, id_char, delay);
    }
    else if (buf[6] == 'd' &&
             buf[7] == 'e' &&
             buf[8] == 'p' &&
             buf[9] == 'a' &&
             buf[10] == 'r' &&
             buf[11] == 't' &&
             buf[12] == 'u' &&
             buf[13] == 'r' &&
             buf[14] == 'e' &&
             buf[15] == ' ')
    {
      int index = 16;
      while (buf[index] == ' ')
        index = index + 1;
      char id_char[100];
      int id_char_index = 0;
      while (buf[index] != '\0' && buf[index] != ' ' && buf[index] != '\n')
      {
        id_char[id_char_index] = buf[index];
        id_char_index = id_char_index + 1;
        index = index + 1;
      }
      id_char[id_char_index] = '\0';
      while (buf[index] == ' ')
        index = index + 1;
      char delay[100];
      int delay_index = 0;
      while (buf[index] != '\0' && buf[index] != ' ' && buf[index] != '\n')
      {
        delay[delay_index] = buf[index];
        delay_index = delay_index + 1;
        index = index + 1;
      }
      delay[delay_index] = '\0';
      printf("OK\n");
      delay_departure_train(root, id_char, delay);
    }
    strcpy(reply, "Delayed successfully\n");
  }
  xmlSaveFormatFileEnc("trains.xml", doc, "UTF-8", 0);
  xmlFreeDoc(doc);
  xmlCleanupParser();

  printf("[Thread %d]Trimitem mesajul inapoi...%s\n", idThread, reply);

  /* returnam mesajul clientului */
  if (write(cl, reply, sizeof(reply)) <= 0)
  {
    printf("[Thread %d] ", idThread);
    perror("[Thread]Eroare la write() catre client.\n");
  }
  else
    printf("[Thread %d]Mesajul a fost transmis cu succes.\n", idThread);
}
