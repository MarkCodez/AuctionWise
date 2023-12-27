/*Mark Eskander
Client file
Final project - CSCI 3240
allows client to get information from data file
to auction on items
04/30/2023
*/

#include "csapp.h"
#include "csapp.c"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// function prototypes
void verifyClient(int clientfd);
void beginService(int clientfd);
void displayItems(int clientfd);
void bidItems(int clientfd);
// void connectClient(int clientfd);

int main(int argc, char **argv)
{
	int clientfd;
	char *host, *port;
	rio_t rio;

	if (argc != 3) 
	{
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(0);
	}
	host = argv[1];
	port = argv[2];

    // initiate connection request with server
	clientfd = Open_clientfd(host, port);
	Rio_readinitb(&rio, clientfd);
    
    // make sure client is a legal bidder
    verifyClient(clientfd);
    
    printf("\nGoodbye! Thank you for choosing AucitonWise\n");
    
    return 0;
}

void verifyClient(int clientfd)
{
    rio_t rio;
    char buf[MAXLINE];
    
    printf("Server connected. \nTo continue, please enter your AW number for verification or 0 to quit. (i.e. aw1234)\n");
	printf("[AW number or 0]: ");
    
    while (Fgets(buf, MAXLINE, stdin) != NULL)
    {
        char userName[MAXLINE];
        
        // user decides to quit
        if (buf[0] == '0')
		{
            Rio_writen(clientfd, buf, strlen(buf));// choice
			break;
		}
        else
        {
            char found[1];
            
            Rio_writen(clientfd, buf, strlen(buf));// aw userID
            
            // reading from server
            Rio_readinitb(&rio, clientfd);
            Rio_readlineb(&rio, found, 2);
            found[strcspn(found, "\n")] = '\0';
            
            // user found
            if (found[0] == '1')
            {
                Rio_readlineb(&rio, userName, MAXLINE);
                userName[strcspn(userName, "\n")] = '\0';
                printf("\nValid user, %s found. \nWelcome to AuctionWise!\n", userName);
                beginService(clientfd);
                return;
            }
            // invalid name
            else
            {
                printf("\nUnable to verify. Re-enter [AW number or 0]: ");
            }
        }
    }
}    

void beginService(int clientfd)
{
    rio_t rio;
    char buf[MAXLINE];
    
    // prompt user whether they want to see items or to bid
    printf("\nWhat would you like to do? \n[0]Terminate\n[1]View items\n[2]Bid\n");
    printf("Enter a choice [0, 1, 2]: ");
    
    while (Fgets(buf, MAXLINE, stdin) != NULL)
    {
        // quit
        if (buf[0] == '0')
        {
            Rio_writen(clientfd, buf, strlen(buf));// choice
            return;
        }
        // display
        else if(buf[0] == '1')
        {
            Rio_readinitb(&rio, clientfd);
            Rio_writen(clientfd, buf, strlen(buf));// choice
            displayItems(clientfd);
        }
        // bid
        else if (buf[0] == '2')
        {
            Rio_writen(clientfd, buf, strlen(buf));// choice
            bidItems(clientfd);
            
            Rio_writen(clientfd, "1", 1);
            displayItems(clientfd);
        }
        // invalid choice
        else
        {
            printf("\nInvalid choice.");
        }
        printf("\n\nChoose ->\n[0]Terminate\n[1]View items\n[2]Bid\n");
        printf("Enter a choice [0, 1, 2]: ");
    }
}

void displayItems(int clientfd)
{
    rio_t rio;
    char Continue[1];
    char itemInfo[MAXLINE];
    
    // reading from server
    Rio_readinitb(&rio, clientfd);
    Rio_readlineb(&rio, Continue, 2);
    Continue[strcspn(Continue, "\n")] = '\0';
    
    // read in items, line-by-line
    if (Continue[0] == '1')
    {
        while (Continue[0] == '1')
        {
            memset(itemInfo, 0, sizeof(itemInfo)); // clear content
            
            // reading from server
            Rio_readlineb(&rio, itemInfo, MAXLINE);
            itemInfo[strcspn(itemInfo, "\n")] = '\0';

            // display information
            printf("\n%s", itemInfo);

            // more records or not
            Rio_readlineb(&rio, Continue, 2);
            Continue[strcspn(Continue, "\n")] = '\0';
        }
    }
}

void bidItems(int clientfd)
{
    rio_t rio;
    char itemExists[1];
    char item[MAXLINE];
    char itemPrice[MAXLINE];
    char userBid[MAXLINE];
    char name[MAXLINE];
    
    // keep looping until user enters valid item name
    while (1)
    {
        printf("\nEnter name of item you would like to bid on: ");
        Fgets(item, MAXLINE, stdin);
        strcpy(name, item);
        name[strcspn(name, "\n")] = '\0';
        
        Rio_writen(clientfd, item, strlen(item)); // item name
        // reading from server
        Rio_readinitb(&rio, clientfd);
        Rio_readlineb(&rio, itemExists, 2);
        itemExists[strcspn(itemExists, "\n")] = '\0';
        
        // if item found, allow bidding
        if (itemExists[0] == '1')
        {
            Rio_readlineb(&rio, itemPrice, MAXLINE);
            itemPrice[strcspn(itemPrice, "\n")] = '\0';

            // Rio_readinitb(&rio, clientfd);
            printf("\nMinimum bid for %s must be greater than $%s.\n", name, itemPrice);
            printf("Enter your bid: ");
            Fgets(userBid, MAXLINE, stdin);

            int bid = atoi(userBid);
            int minPrice = atoi(itemPrice);

            // bid price must be greater than minimum
            while(bid <= minPrice)
            {
                printf("\nInvalid bid. Minimum bid for %s must be greater than $%s.\n", name, itemPrice);
                printf("Enter your bid: ");
                Fgets(userBid, MAXLINE, stdin);

                bid = atoi(userBid);
            }

            Rio_writen(clientfd, userBid, strlen(userBid)); // user bid
            userBid[strcspn(userBid, "\n")] = '\0';
            
            printf("\nBid successful. New minimum bid for %s is $%s.", name, userBid);

            break;
        }
        // item not found
        else if(itemExists[0] == '0')
        {
            printf("\nInvalid item name.");
        }
    }
}   
