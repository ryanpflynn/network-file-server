/*************************
 * Ryan Flynn
 * Network File Transfer Server
 **************************/
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include "Winsock2.h"
#include <dirent.h>
#include "Socket.h"

using namespace std;

void getFolderContents(Socket&);
bool checkName(string, string);
void getFileReady(char*, Socket&);
void checkCommand(char*, Socket&);
char* stringToCharArray(string);

int main(int argc, char * argv[])
{
        //Variables initialization:
        int port = 54321;
        string ipAddress;
        string id = "user";
        bool done = false;
        char recMessage[STRLEN];

        //Server
        ServerSocket sockServer;
        cout<<"Server started..."<<endl;
        sockServer.StartHosting( port );

        while ( true )
        {
                //Protocol to initiate communication with the client:
                cout << "Prompting for login..." <<endl;
                sockServer.SendData(stringToCharArray("LOGIN"));
                sockServer.RecvData( recMessage, STRLEN );

                if(checkName(recMessage, id) == false) {
                        cerr << "Invalid User ID: '" << recMessage <<"'"  << endl;
                        sockServer.SendData(stringToCharArray("UNWELCOME"));
                        sockServer.CloseConnection();
                        sockServer.Listen();

                }else{
                        cout << "User '"<< recMessage << "' logged in successfully." << endl;
                        sockServer.SendData(stringToCharArray("WELCOME"));

                        while(!done) {
                                sockServer.RecvData( recMessage, STRLEN );
                                cout<<"Received: > "<<recMessage<<endl;

                                if ( strcmp( recMessage, "QUIT")==0 ) {
                                        sockServer.CloseConnection();
                                        sockServer.Listen();
                                        //cout << "Connection closed. Exiting the program" << endl;
                                        //return 0;
                                        //done = true;

                                }else{
                                        checkCommand(recMessage, sockServer);
                                }
                        }

                }

        }

        return 0;
}


/********************
* Name: checkName
* Purpose: Verify user login
* Arguments: required name, user name
* Returns: true if matched, false otherwise
********************/
bool checkName(string name, string user){

        if(name == user) {
                return true;
        }else{
                return false;
        }
}

/********************
* Name: checkCommand
* Purpose: Parse message
*    and execute operations
* Arguments: message and socket ptr
* Returns: nothing
********************/
void checkCommand(char* command, Socket& sockServer){

        if ( strncmp( command, "LIST", 4 )==0) {

                getFolderContents(sockServer);

        }
        else if ( strncmp( command, "SEND", 4 )==0) {

                getFileReady(command, sockServer);

        }
        else if ( strncmp( command, "EOF OK", 6 )==0) {

                cout << "---File Transmitted Successfully---" << endl;
                sockServer.SendData(stringToCharArray("OK"));

        }
        else{

                sockServer.SendData(stringToCharArray("ERROR"));

        }

}
/********************
* Name: getFolderContents
* Purpose: reads file contents
*     of program directory and sends
*     a list of file to the client
* Arguments: socket
* Returns: nothing
********************/
void getFolderContents(Socket& sockServer){
        char dirList [STRLEN];
        char num [STRLEN];
        char newline [2];
        char space [2];
        newline[0]='\n';
        newline[1]='\0';
        space[0]='\t';
        space[1]='\0';

        //Clear out dirList
        memset(&dirList[0], NULL, sizeof(dirList));

        DIR *pdir = NULL; // remember, it's good practice to initialize a pointer to NULL!
        pdir = opendir ("."); // "." will refer to the current directory
        struct dirent *pent = NULL;

        if (pdir == NULL) // if pdir wasn't initialized correctly
        { // print an error message and exit the program
                cerr << "Error: Directory could not be initialized correctly." <<endl;
                exit (3);
        } // end if

        int i = 1;
        while (pent = readdir (pdir)) // while there is still something in the directory to list
        {
                if (pent == NULL) // if pent has not been initialized correctly
                { // print an error message, and exit the program
                        cerr << "Error: Directory could not be initialized correctly." <<endl;
                        exit (4);
                }

                //Build String with Folder Contents
                strcat(dirList, itoa(i,num,10));
                strcat(dirList, space);
                strcat(dirList,pent->d_name);
                strcat(dirList, newline);

                i++;
        }
        sockServer.SendData(dirList);
        // finally, let's close the directory
        closedir (pdir);

        for (int i=0; i<STRLEN; i++)
                dirList[i]=NULL;
}
/********************
* Name: getFileReady
* Purpose: fetches file name
*  and passes it to the send
*  method
* Arguments: filename, socket
* Returns: nothing
********************/
void getFileReady(char *message, Socket& sockServer){

        //parse SEND message for file number
        string toParse(message);
        string fileName;
        vector<string> dirContents;
        int space = toParse.find(' ');
        int fileNum;

        if (space == -1) {
                cerr << "Invalid File Request" << endl;
        }else{
                string number = toParse.substr((space+1), toParse.length()-space+1);
                fileNum = atoi(number.c_str());
                cout << "File Number "<< fileNum << " Requested." << endl;
        }

        DIR *pdir = NULL;
        pdir = opendir ("."); // "." current directory
        struct dirent *pent = NULL;

        if (pdir == NULL)
        {
                cerr << "Error: Directory could not be initialized correctly." <<endl;
                exit (3);
        }

        while (pent = readdir(pdir)) // while there is still something in the directory to list
        {
                if (pent == NULL)
                {
                        cerr << "Error: Directory could not be initialized correctly." <<endl;
                        exit (4);
                }

                dirContents.push_back(pent->d_name);

        }
        closedir (pdir);

        //Check for number in Range!
        if(fileNum < 0 || fileNum > dirContents.size()) {

                sockServer.SendData(stringToCharArray("ERROR"));

        }else{

                fileName = dirContents[fileNum-1];

                //send file

                sockServer.SendFile(stringToCharArray(fileName));
                sockServer.SendData(stringToCharArray("EOFEOFEOFEOFEOFEOF"));
        }

        for (int i=0; i <  dirContents.size(); i++)
                dirContents[i]=" ";
}

/********************
* Name: stringToCharArray
* Purpose: Converts a string in a character array to send
   via the socket.
* Arguments: String to be converted
* Returns: New Cstring.
********************/

char* stringToCharArray(string oldStr){

        char *newCStr = new char[oldStr.size()+1];
        strcpy(newCStr, oldStr.c_str());

        return newCStr;
}
