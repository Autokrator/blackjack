#include "socketserver.h"
#include "socket.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <exception>
#include <mutex>
#include <algorithm>

int PORT = 2200; // Port to listen for active connections

// Function used to respond to a specific client
void respondThread(Communication::Socket * , bool *kill);

// Function used to spawn threads that run respondThread()
void connectionThread(bool* kill);

// Function used to trigger the SocketServer to shutdown
void triggerShutdown(bool *kill, Communication::SocketServer *c);

int main(){
    bool kill = false; // Used to terminate all active threads
    std::string killResponse; // Used to initate termination process

    // The main thread only spawns one thread that manages all connections to
    // the server by dynamically spawning "response threads"
    std::thread *connection = new std::thread(connectionThread, &kill);

    do{
        std::cout<<"Do you want to close the server?\n";
        std::cin >> killResponse;
        if(killResponse == "yes"){
            kill = true;
            break;
        }
    } while(true);

    connection->join();
    delete connection;
    return 0;
}

void respondThread(Communication::Socket *socket, bool *kill) {

    Communication::ByteArray message(""); // Stores received messages

    do{ // Loop to respond to client while is active (does not send "done")
    std::cout << "loop \n";

        int readCode; // Holds the return value of Communication::Socket::Read()
        std::string response; // Holds the received message as a string
        try{
            readCode = socket->Read(message); // Wait for response
            response = message.ToString(); // Store response as string
        } catch(std::exception e) { // Incase message exceeds buffer size
            response = "Buffer Size Exceeded!";
            socket->Write(Communication::ByteArray(response)); // Error response
            continue; // Skip the rest and listen for a response from client
        }

        if(!response.compare("done")) // Exit condition
            break;

        // Change the response to upper case
        std::transform(response.begin(),response.end(),
                       response.begin(), ::toupper);

        // Send response to client
        socket->Write(Communication::ByteArray(response));

    } while(!(*kill));

    socket->Close(); // Close the connection
    delete socket; // Free allocated memory created by connectionThread
    socket = NULL;

}

void connectionThread(bool* kill){
    std::vector<std::thread *> rThreads; // Currently active response threads

    // Start listening for incoming connections on specified port
    Communication::SocketServer connectionSocket(PORT);

    // Create a shutdown thread to terminate a possible block with Accept()
    std::thread shutdown_thread(triggerShutdown, kill, &connectionSocket);

    while(true){ // Manage incoming connections loop
        // Create a temporary Communication::Socket to pass to respondThread
        Communication::Socket *socket;

        try{
            // Blocks until a connection is attempted
            socket = new Communication::Socket(connectionSocket.Accept());
        } catch(int e) {
            if(e == 2) // Catch TerminationException
                break; // Exit loop
            else
                continue; // Try again without creating new response thread
        }

        // Create a response thread for each incoming connection and add to vector
        std::thread *new_thread = new std::thread(respondThread, socket, kill);
        rThreads.push_back(new_thread);
    }

    // Join shutdown thread
    shutdown_thread.join();

    // Join all response threads in rThreads
    for(auto&& thread : rThreads)
        thread->join();

    // Clean up allocated memory
    while(!rThreads.empty()){
        delete rThreads.back();
        rThreads.pop_back();
    }

    std::cout << "Closed Connection. Exiting...\n";
}

void triggerShutdown(bool *kill, Communication::SocketServer *c){
    int refresh_interval = 500; // refresh interval to check for termination
    while(!(*kill)) // Checks for termination
        std::this_thread::sleep_for(std::chrono::milliseconds(refresh_interval));

    // Shutdown SocketServer
    c->Shutdown();
}
