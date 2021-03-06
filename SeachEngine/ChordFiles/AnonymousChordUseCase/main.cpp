#include <iostream>
#include <string>
#include <stdlib.h>
#include <anonCh/ChordNode.h>
#include <pthread.h>
#include <anonCh/ProtocolSingleton.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>
#include <openssl/ssl.h>

using namespace std;

Node *node = NULL;
ChordNode *chord = NULL;

/* portul folosit */
#define PORT 5757

/* codul de eroare returnat de anumite apeluri */
    extern int errno;

void handle_sigchld(int sig) {
    while (waitpid((-1), 0, WNOHANG) > 0);

}

void child(int client, char * const argv[]);

int main(int argc, char * const argv[]) {
    // This application receives args, "ip", "port", "overlay identifier (unique string)", "root directory)"
    chord = P_SINGLETON->initChordNode(std::string(argv[1]), atoi(argv[2]), std::string("AnonymousChord"), std::string("/home/ciprian/Desktop/SearchEngine"));
    node = new Node(std::string(argv[3]), 443);
    chord->join(node);

//    Query *query = new Query(2);
//    query->setOwnerIP(chord->getThisNode()->getIp());
//
//    chord->addHandledQuery(query);
//
//    string last_node_key;
//    string last_node_iv;
//
//    chord->getNodeKey(chord->getThisNode(), query, last_node_key, last_node_iv);
//    string crypt = chord->crypt("ana", reinterpret_cast<unsigned char*> ((char*) last_node_key.c_str()), reinterpret_cast<unsigned char*> ((char*) last_node_iv.c_str()));
//    string plain = chord->decrypt(crypt, reinterpret_cast<unsigned char*> ((char*) last_node_key.c_str()), reinterpret_cast<unsigned char*> ((char*) last_node_iv.c_str()));

    struct sockaddr_in server; // structura folosita de server
    struct sockaddr_in from;
    int sd; //descriptorul de socket 

    /* crearea unui socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    int optval = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof ( optval));

    /* pregatirea structurilor de date */
    bzero(&server, sizeof (server));
    bzero(&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons(PORT);

    /* atasam socketul */
    if (bind(sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 5) == -1) {
        perror("[server]Eroare la listen().\n");
        return errno;
    }

    /* servim in mod iterativ clientii... */
    while (1) {
        int client;
        socklen_t length = sizeof (from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        client = accept(sd, (struct sockaddr *) &from, &length);

        /* eroare la acceptarea conexiunii de la un client */
        if (client < 0) {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        int copil_fct;
        if ((copil_fct = fork()) == 0) {
            child(client, argv);
        }
        if (signal(SIGCHLD, handle_sigchld) == SIG_ERR) {
            perror(0);
            exit(1);
            printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
}

void child(int client, char * const argv[]) {

        char comanda[100];
        char key[1100];
        bzero(key, 1100);
        bzero(comanda, 100);
        fflush(stdout);


        if (read(client, comanda, 100) <= 0) {
            perror("[server]Eroare la read() de la client.\n");
            close(client); /* inchidem conexiunea cu clientul */
            return; /* continuam sa ascultam */
        }

        if (strcmp(comanda, "exit") == 0) {
            delete node;
            delete chord;
            close(client);
            return;
        }

        if (strcmp(comanda, "get") == 0) {

            if (read(client, key, 1100) <= 0) {
                perror("[server]Eroare la read() de la client.\n");
                close(client); /* inchidem conexiunea cu clientul */
                return; /* continuam sa ascultam */
            }

            string value = chord->get(string(key));

            if (write(client, value.c_str(), 5100) <= 0) {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
        } else
            if (strcmp(comanda, "put") == 0) {


            if (read(client, key, 1100) <= 0) {
                perror("[server]Eroare la read() de la client.\n");
                close(client); /* inchidem conexiunea cu clientul */
                return; /* continuam sa ascultam */
            }

            char value[5100];
            if (read(client, &value, sizeof ( value)) <= 0) {
                perror("[AnonCH.Server]Error at client read.\n");
                return;
            }

            chord->put(key, string(value));
        } else
            if (strcmp(comanda, "remove") == 0) {

            if (read(client, key, 1100) <= 0) {
                perror("[server]Eroare la read() de la client.\n");
                close(client); /* inchidem conexiunea cu clientul */
                return; /* continuam sa ascultam */
            }

            chord->removekey(key);
        } else
            if (strcmp(comanda, "randomWalk") == 0) {

            if (read(client, key, 1100) <= 0) {
                perror("[server]Eroare la read() de la client.\n");
                close(client); /* inchidem conexiunea cu clientul */
                return; /* continuam sa ascultam */
            }

            string value = chord->randomWalk(string(key));

            if (write(client, value.c_str(), 5100) <= 0) {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
        } else
            if (strcmp(comanda, "getallnodes") == 0) {
            vector<Node*> nodeFingerTable = chord->getFingerTable();

            string value;

            vector<Node*>::iterator it;

            for (it = nodeFingerTable.begin(); it != nodeFingerTable.end(); it++) {
                if (value.find((*it)->getIp()) == string::npos)
                    value = value + " " + (*it)->getIp();
            }

            char buff[100];
            string length = string(chord->itoa(value.length(), buff, 10));

            if (write(client, length.c_str(), 8) <= 0) {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }

            if (write(client, value.c_str(), value.length()) <= 0) {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
        close(client);
    }
}
