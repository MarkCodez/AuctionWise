/*Mark Eskander
Server file
Project 4 - CSCI 3240
04/30/23
Has access to information stored in items.txt 
and Awusers.txt file
*/

#include "csapp.h"
#include "csapp.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// function prototypes
void *thread(void *vargp);
bool verifyClient(int connfd);
void provideService(int connfd);
void changeBidPrice(char *userBidPrice, char *itemName);
// void accessFile(int connfd);

// struct for thread function arguments
struct threadArguments
{
    char client_hostname[MAXLINE], client_port[MAXLINE]; 
    int connfd;
};

int main(int argc, char **argv)
{
	int listenfd;
    struct threadArguments *argPtr;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
    pthread_t tid;

	if (argc != 2) 
	{
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}

	listenfd = Open_listenfd(argv[1]);

    
	while (1) 
	{
		clientlen = sizeof(struct sockaddr_storage);
        argPtr = Malloc(sizeof(struct threadArguments));// allocate memory for each newly accepted connection request
		argPtr -> connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);// accept request and assign file descriptor
		Getnameinfo((SA *) &clientaddr, clientlen, argPtr -> client_hostname, MAXLINE, argPtr -> client_port, MAXLINE, 0);
		printf("Connected to (%s, %s)\n", argPtr -> client_hostname, argPtr -> client_port);
        Pthread_create(&tid, NULL, thread, argPtr);// create thread, call thread function, and pass struct
	}
	return 0;
}

// thread routine, each thread is 
// independent of other threads
void *thread(void *vargp)
{
    struct threadArguments *argPtr;
    argPtr = vargp;
    
    bool validClient;
    
    Pthread_detach(pthread_self());
    validClient = verifyClient(argPtr -> connfd);
    
    // if client can be verified
    if (validClient)
    {
        provideService(argPtr -> connfd); // service function
    }
    
    Close(argPtr -> connfd); // close connection when user terminates
    printf("(%s, %s) disconnected\n", argPtr -> client_hostname, argPtr -> client_port);
    Free(vargp); // free memory allocated to thread struct
    return NULL;
}

bool verifyClient(int connfd)
{
    FILE *filePtr;
    size_t n;
    rio_t rio;
    char userChoice[MAXLINE];
    
    Rio_readinitb(&rio, connfd);
    
	// keep looping until userChoice is 0 or valid user found
	while ((n = Rio_readlineb(&rio, userChoice, MAXLINE)) != 0)
    {
        userChoice[strcspn(userChoice, "\n")] = '\0';
        
        char userName[MAXLINE];
        char userID[MAXLINE];
        
        // quit program
        if (userChoice[0] == '0')
        {
            return false;
        }
        // check for user
        else
        {
            // to hold 1 line of text from file
            char bufferArray[100];
            
            // open file for reading
			filePtr = fopen("AWusers.txt", "r");
            
            // ensure file opened correctly
            if (filePtr == NULL)
            {
                printf("Error opening file.\n");
                exit(0);
            }
            
            // read file line-by-line
            while (fgets(bufferArray, 100, filePtr) != NULL)
            {
                char *token = strtok(bufferArray, ",");
                
                // parse each line and extract the fields
				strcpy(userID, token);
				token = strtok(NULL, ",");
				strcpy(userName, token);
				token = strtok(NULL, ",");
                
                // if valid user match return true
                if (strcmp(userChoice, userID) == 0)
                {
                    Rio_writen(connfd, "1", 1); // found signal
                    Rio_writen(connfd, userName, strlen(userName)); // user name
                    
                    // close the file
                    fclose(filePtr);
                    
                    return true;
                }
            }
            
            // close the file
			fclose(filePtr);
            
            Rio_writen(connfd, "0", 1);
        }
        Rio_readinitb(&rio, connfd);
    }
    return false;
}

void provideService(int connfd)
{
    FILE *filePtr;
    size_t n;
    rio_t rio;
    char userChoice[MAXLINE];
    
    Rio_readinitb(&rio, connfd);
    
    // keep looping until userChoice is 0
	while ((n = Rio_readlineb(&rio, userChoice, MAXLINE)) != 0)
    {
        // quit service
        if (userChoice[0] == '0')
        {
            return;
        }
        else if(userChoice[0] == '1')
        {
            // to hold 1 line of text from file
            char bufferArray[100];
            
            // open file for reading
			filePtr = fopen("items.txt", "r");
            
            // ensure file opened correctly
            if (filePtr == NULL)
            {
                printf("Error opening file.\n");
                exit(0);
            }
            
            // read file line-by-line
            while (fgets(bufferArray, 100, filePtr) != NULL)
            {
                Rio_writen(connfd, "1", 1);
                Rio_writen(connfd, bufferArray, strlen(bufferArray)); // item information
            }
            
            Rio_writen(connfd, "0", 1);
        }
        else if(userChoice[0] == '2')
        {
            while(1)
            {
                Rio_readinitb(&rio, connfd);
                
                bool isFound = false;
                char userItemName[MAXLINE];
                char itemName[MAXLINE];
                char itemPrice[MAXLINE];
                char userBidPrice[MAXLINE];
                char bufferArray[100];

                Rio_readlineb(&rio, userItemName, MAXLINE); // user item name
                userItemName[strcspn(userItemName, "\n")] = '\0';

                // open file for reading
                filePtr = fopen("items.txt", "r");

                // ensure file opened correctly
                if (filePtr == NULL)
                {
                    printf("Error opening file.\n");
                    exit(0);
                }

                // read file line-by-line
                while (fgets(bufferArray, 100, filePtr) != NULL)
                {
                    char *token = strtok(bufferArray, ",");

                    // parse each line and extract the fields
                    strcpy(itemName, token);
                    token = strtok(NULL, ",");
                    strcpy(itemPrice, token);

                    // if valid user match return true
                    if (strcmp(userItemName, itemName) == 0)
                    {
                        Rio_writen(connfd, "1", 1);
                        isFound = true;

                        Rio_writen(connfd, itemPrice, MAXLINE);

                        Rio_readinitb(&rio, connfd);
                        Rio_readlineb(&rio, userBidPrice, MAXLINE); // user bid
                        userBidPrice[strcspn(userBidPrice, "\n")] = '\0';
                        
                        // close the file
                        fclose(filePtr);
                        
                        changeBidPrice(userBidPrice, itemName);

                        break;
                    }
                }

                //no matching item found
                if (!isFound)
                {
                    Rio_writen(connfd, "0", 1);
                }
                else
                {
                    break;
                }
            }
        }
    }
    // close the file
	fclose(filePtr);
}               
 
void changeBidPrice(char *userBidPrice, char *itemName)
{
    FILE* filePtr;
    char line[MAXLINE];
    char temp[MAXLINE];
    char fileInfo[MAXLINE];
    char lineToChange[MAXLINE];
    char item[MAXLINE];
    char price[MAXLINE];
    char owner[MAXLINE];
    int found = 0;
    
    memset(fileInfo, 0, sizeof(fileInfo)); // clear content

    // Open the file for reading
    filePtr = fopen("items.txt", "r");
    if (filePtr == NULL) 
    {
        printf("Error: Failed to open file.\n");
        exit(1);
    }

    // read file line-by-line
    while (fgets(line, 100, filePtr) != NULL)
    {
        strcpy(temp, line); // store copy of line
        
        char *token = strtok(line, ",");
        strcpy(item, token);
        token = strtok(NULL, ",");
        strcpy(price, token);
        token = strtok(NULL, ",");
        strcpy(owner, token);
        
        // Check if the line contains the item we want to update
        if (strcmp(item, itemName) == 0) 
        {
            sprintf(lineToChange, "%s,%s,%s", item, userBidPrice, owner);
            found = 1;
        }
        
        // no change to line
        if (!found)
        {
            strcat(fileInfo, temp);
        }
        found = 0;
    }

    fclose(filePtr);

    // Write the updated contents back to the file
    filePtr = fopen("items.txt", "w");
    if (filePtr == NULL) 
    {
        printf("Error: Failed to open file.\n");
        exit(1);
    }
    
    // Write the updated contents to the file
    fprintf(filePtr, "%s%s", fileInfo, lineToChange);

    fclose(filePtr);
}
