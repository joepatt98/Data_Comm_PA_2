// Authors: Joseph Patterson and Stephanie Shisler
// NetIDs: jhp232 and sas880

#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>   // defines types (like size_t)
#include <sys/socket.h>  // defines socket class
#include <netinet/in.h>  // defines port numbers for (internet) sockets, some address structures, and constants
#include <netdb.h>
#include <fstream>
#include <arpa/inet.h>   // if you want to use inet_addr() function
#include "packet.h"
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <sstream>

using namespace std;

int main(int argc, char *argv[]){

  char filename[512];
  strcpy(filename, argv[4]);

  struct hostent *s;
  s = gethostbyname(argv[1]);

  int mysocket = 0;

  // sets up UDP socket for file transmission
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0))==-1) {

    cout << "Error in creating UDP socket.\n";
    exit(1);

  }

  struct sockaddr_in server;
  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(atoi(argv[2]));
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  socklen_t slen = sizeof(server);
  if (bind(mysocket, (struct sockaddr *)&server, slen) == -1) {

    cout << "Error in binding.\n";
    exit(1);

  }

  struct sockaddr_in emulator;
  memset((char *) &emulator, 0, sizeof(emulator));
  emulator.sin_family = AF_INET;
  emulator.sin_port = htons(atoi(argv[3]));
  emulator.sin_addr.s_addr = htonl(INADDR_ANY);
  bcopy((char *) s->h_addr, (char *) &emulator.sin_addr.s_addr, s->h_length);
  socklen_t elen = sizeof(emulator);

  ofstream arrival_log("arrival.log", ios_base::out | ios_base::trunc);

  char payload[512];
  string payload_data = "";
  bool continue_loop = true;

  while (continue_loop) {

      memset(payload, 0, sizeof(payload));

      int bytes_received = recvfrom(mysocket, payload, sizeof(payload), 0,
        (struct sockaddr *) &server, &slen);

      if (bytes_received > 0) {

          packet *rcv_packet = new packet(0, 0 , 30, payload);
          rcv_packet->deserialize(payload);

          arrival_log << rcv_packet->getSeqNum() << endl;

          switch (rcv_packet->getType()) {

              case 1: {

                payload_data += rcv_packet->getData();

                packet *spacket;
                spacket = new packet(0,rcv_packet->getSeqNum(),0,NULL);

                memset(payload,0,sizeof(payload));
                spacket->serialize(payload);

                if (sendto(mysocket, payload, sizeof(payload), 0,
                  (struct sockaddr *) &emulator, elen) == -1) {

                      cout << "Error in sendto function.\n";
                      exit(1);

                }

                break;

              }

              case 3: {

                packet *spacket;
                spacket = new packet(2,rcv_packet->getSeqNum(),0,NULL);

                memset(payload,0,sizeof(payload));
                spacket->serialize(payload);

                if (sendto(mysocket, payload, sizeof(payload), 0,
                  (struct sockaddr *) &emulator, elen) == -1) {

                      cout << "Error in sendto function.\n";
                      exit(1);

                }

                continue_loop = false;
                break;

              }

              default: {

                  cout << "Error Occurred\n";
                  continue_loop = false;
                  break;

              }

              continue_loop = false;
              break;

          }

      }

      else {

          cout << "Failed to receive.\n";
          exit(1);

      }

  } // end while loop

  ofstream output_file(filename);
  output_file << payload_data;
  output_file.close();

  arrival_log.close();

  close(mysocket);

  return 0;

}
