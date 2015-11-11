# Logger
This is university lab work. It's purpose was to learn how to work with sockets with UDP protocol.

Requests:
-Send message with one of 3 levels(warn, info, error) and text
-Server should also log date and time of recieve, and ip adress of sender
-Server should return you all messages: 
  *by level
  *by date
  *by time interval
-Delete all mesages

Command to compile server :

'''g++ -o server -std=c++11 server.cpp'''

and run server :
''' ./server '''

For client:

'''g++ -o client client.cpp'''
'''./client'''
