//Socket.cpp
#include "Socket.h"

Socket::Socket()
{
    if( WSAStartup( MAKEWORD(2, 2), &wsaData ) != NO_ERROR )
    {
        cerr<<"Socket Initialization: Error with WSAStartup\n";
        system("pause");
        WSACleanup();
        exit(10);
    }

    //Create a socket
    mySocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( mySocket == INVALID_SOCKET )
    {
        cerr<<"Socket Initialization: Error creating socket"<<endl;
        system("pause");
        WSACleanup();
        exit(11);
    }

    myBackup = mySocket;
}

Socket::~Socket()
{
    WSACleanup();
}

bool Socket::SendData( char *buffer )
{
    send( mySocket, buffer, strlen( buffer ), 0 );
    return true;
}

bool Socket::RecvData( char *buffer, int size )
{
    int i = recv( mySocket, buffer, size, 0 );
    buffer[i] = '\0';
    return true;
}

void Socket::CloseConnection()
{
    //cout<<"CLOSE CONNECTION"<<endl;
    closesocket( mySocket );
    mySocket = myBackup;
}

void Socket::GetAndSendMessage()
{
    char message[STRLEN];
    cin.ignore();//without this, it gets the return char from the last cin and ignores the following one!
    cout<<"Send > ";
    cin.get( message, STRLEN );
    SendData( message );
}

void ServerSocket::StartHosting( int port )
{
     Bind(port);
     Listen();
}

void ServerSocket::Listen()
{
    //cout<<"LISTEN FOR CLIENT..."<<endl;

    if ( listen ( mySocket, 1 ) == SOCKET_ERROR )
    {
        cerr<<"ServerSocket: Error listening on socket\n";
        system("pause");
        WSACleanup();
        exit(15);
    }

    // ****************
    struct sockaddr_in clientSocket;
    clientSocket.sin_family = AF_INET;
    int c_len = sizeof(clientSocket);
    // ****************

    //cout<<"ACCEPT CONNECTION..."<<endl;
    acceptSocket = accept( myBackup, (SOCKADDR *) &clientSocket, &c_len );
    //acceptSocket = accept( myBackup, NULL, NULL );
    while ( acceptSocket == SOCKET_ERROR )
    {
        acceptSocket = accept( myBackup, (SOCKADDR *) &clientSocket, &c_len );
        //acceptSocket = accept( myBackup, NULL, NULL );
    }
    mySocket = acceptSocket;

    cout << "Connection Accepted. " << endl;

    // *****************
    unsigned long ulAddr = clientSocket.sin_addr.s_addr;
    //char *some_addr;
    char *some_addr;
    some_addr = inet_ntoa(clientSocket.sin_addr);
//    InetNtop(AF_INET, clientSocket.sin_addr.s_addr, some_addr, 100);
    //printf("address is      %s \n", some_addr);
    // *****************

}

void ServerSocket::Bind( int port )
{
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = inet_addr( "0.0.0.0" );
    myAddress.sin_port = htons( port );

    //cout<<"BIND TO PORT "<<port<<endl;

    if ( bind ( mySocket, (SOCKADDR*) &myAddress, sizeof( myAddress) ) == SOCKET_ERROR )
    {
        cerr<<"ServerSocket: Failed to connect\n";
        system("pause");
        WSACleanup();
        exit(14);
    }
}

void ClientSocket::ConnectToServer( const char *ipAddress, int port )
{
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = inet_addr( ipAddress );
    myAddress.sin_port = htons( port );

    //cout<<"CONNECTED"<<endl;

    if ( connect( mySocket, (SOCKADDR*) &myAddress, sizeof( myAddress ) ) == SOCKET_ERROR )
    {
        cerr<<"ClientSocket: Failed to connect\n";
        system("pause");
        WSACleanup();
        exit(13);
    }
}
void Socket::ReceiveFile(char *filename)
{

		char rec[50] = "";
	//recv( mySocket, filename, 32, 0 );
	//New code:
	int i = recv( mySocket, filename, 256, 0 );
	filename[i]='\0';

	send( mySocket, "OK", strlen("OK"), 0 );

	FILE *fw = fopen(filename, "wb");

	int recs = recv( mySocket, rec, 32, 0 );
	send( mySocket, "OK", strlen("OK"), 0 );

	rec[recs]='\0';
	int size = atoi(rec);

	while(size > 0)
	{
		char buffer[1030];

		if(size>=1024)
		{
			recv( mySocket, buffer, 1024, 0 );
			send( mySocket, "OK", strlen("OK"), 0 );
			fwrite(buffer, 1024, 1, fw);
		}
		else
		{
			recv( mySocket, buffer, size, 0 );
			send( mySocket, "OK", strlen("OK"), 0 );
			buffer[size]='\0';
			fwrite(buffer, size, 1, fw);
		}
		size -= 1024;
	}

	fclose(fw);

}

void Socket::SendFile( char *fpath)
{

	ifstream myFile (fpath, ios::in|ios::binary|ios::ate);
	int size = (int)myFile.tellg();
	myFile.close();

	char filesize[10];itoa(size,filesize,10);


	this->SendData(fpath);
	char rec[32] = "";
	this->RecvData(rec,32);

    this->SendData(filesize);
	this->RecvData(rec,32);


	FILE *fr = fopen(fpath, "rb");

	while(size > 0)
	{
		char buffer[1030];

		if(size>=1024)
		{
			fread(buffer, 1024, 1, fr);
			send( mySocket, buffer, 1024, 0 );

			recv( mySocket, rec, 32, 0 );

		}
		else
		{
			fread(buffer, size, 1, fr);
			buffer[size]='\0';
			send( mySocket, buffer, size, 0 );
			recv( mySocket, rec, 32, 0 );
		}

		size -= 1024;

	}

	fclose(fr);


}






