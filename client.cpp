// Authors: Joseph Patterson and Stephanie Shisler
// NetIDs: jhp232 and xxxxxx

#include <iostream>
#include <sys/types.h>   // defines types (like size_t)
#include <sys/socket.h>  // defines socket class
#include <netinet/in.h>  // defines port numbers for (internet) sockets, some address structures, and constants
#include <netdb.h>
#include <fstream>
#include <arpa/inet.h>   // if you want to use inet_addr() function
#include <string.h>
#include <unistd.h>
#include <vector>
#include <sstream>

using namespace std;

int main(int argc, char *argv[]){

  vector <string> argList(argv, argv + argc);

  char server_name[512];
  strcpy(server_name, argList[1].c_str());

  stringstream ss;
  ss << argList[2];
  int n_port;
  ss >> n_port;
  //int n_port = stoi(argList[2]);

  char filename[512];
  strcpy(filename, argList[3].c_str());

  struct hostent *s;
  s = gethostbyname(server_name);

  struct sockaddr_in server;
  int mysocket = 0;
  socklen_t slen = sizeof(server);

  // ********************** Stage #1 ********************** //

  char n_payload[512] = "2020";
  //cout << "Payload contains:  " << n_payload << endl;

  // sets up TCP socket for handshake
  if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    cout << "Error in creating TCP socket.\n";

  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(n_port);
  bcopy((char *)s->h_addr,
	(char *)&server.sin_addr.s_addr,
	s->h_length);

  if (connect(mysocket, (struct sockaddr *)&server, sizeof(server)) == -1)
    cout << "Error in connecting.\n";

  int len, bytes_sent, bytes_received;
  len = strlen(n_payload);
  bytes_sent = send(mysocket, n_payload, len, 0);

  //cout << "Sending Negotiation Handshake... " << n_payload << endl;
  //cout << "Number of bytes sent: " << bytes_sent << endl;
  if (bytes_sent == -1)
    cout << "Error in send function.\n";

  // Now we wait on the response from the server containing the new random port.
  bytes_received = recv(mysocket, n_payload, 512, 0);

  char* r_port = n_payload;

  //cout << "Received the random port: " << r_port << endl;
  //cout << "Number of bytes received: " << bytes_received << endl;
  if (bytes_received == -1)
    cout << "Error in receive function.\n";

  close(mysocket);

  // ********************** Stage #2 ********************** //
  stringstream sss;
  sss << r_port;
  int new_port;
  sss >> new_port;
  //int new_port = stoi(r_port);
  // sets up UDP socket for file transmission
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    cout << "Error in creating UDP socket.\n";

  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(new_port);
  bcopy((char *)s->h_addr,
	(char *)&server.sin_addr.s_addr,
	s->h_length);

  char c;
  string f_contents;
  ifstream file(filename);
  file >> noskipws;
  while ( file >> c ) f_contents += c;
  //file.close();

  char f_payload[f_contents.size() + 1];
	strcpy(f_payload, f_contents.c_str());

  char s_payload[4];
  int length = strlen(f_payload);
  stringstream ssss;
  ssss << length;
  string file_size = ssss.str(); //to_string(length);
  strcpy(s_payload, file_size.c_str());

  //cout << "Sending size of file " << endl;
  //cout << s_payload << endl;
  if (sendto(mysocket, s_payload, strlen(s_payload), 0, (struct sockaddr *)&server, slen)==-1)
    cout << "Error in sendto function.\n";

  //cout << "Sending file " << endl;
  //cout << f_payload << endl;

  if (sendto(mysocket, f_payload, strlen(f_payload), 0, (struct sockaddr *)&server, slen)==-1)
    cout << "Error in sendto function.\n";

  stringstream sssss;
  sssss << s_payload;
  int f_payload_size;
  sssss >> f_payload_size;
  //int f_payload_size = stoi(s_payload);
  char ack[f_payload_size];

  recvfrom(mysocket, ack, f_payload_size, 0, (struct sockaddr *)&server, &slen);
  cout << ack << endl;

  close(mysocket);

  return 0;
}
