#include "socket.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <exception>

std::string SERVER_IP = "127.0.0.1"; // Holds the IP address of the server
int PORT = 2200; // Holds the server port to connect to

int main(){
    // Create a socket object with server information
    Communication::Socket client(SERVER_IP, PORT);

    bool isConnected = false; // Holds if connection has been established
    while(!isConnected){ // Repeat till connection is established
        try{
            client.Open(); // Attempts to establish a connection
            isConnected = true; // If control reaches this point change isConnected
        } catch (std::string e){ // Catch exception thrown by Communication::Socket::Open();
            std::cout << "Failed to connnect. Reattempting to establish a connection"
                      << std::endl;

            // Sleep for 2 seconds before next attempt
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    do{ // Start user input loop
        std::string input; // Holds user input

        std::cout << "Please enter the message to be sent: ";
        std::cin >> input; // Get user input

        // Instanciate a Communication::ByteArray
        Communication::ByteArray message(input);

        int bytes = client.Write(message); // Send Message
        std::cout << "Message Sent. " << bytes << " bytes sent.\n";

        if(input == "done"){ // Exit condition for the loop
            std::cout << "Terminating connection.\n";
            break; // Exit loop
        }

        int readCode; // Holds the return value of Communication::Socket::Read()
        try{
            readCode = client.Read(message); // Wait for response
        } catch(std::exception e) { // Incase message exceeds buffer size
            std::cout << "Error! Buffer size exceeded!\n";
        }
        if(readCode > 0) // Successful read
            std::cout << "Received Message: " << message.ToString()
                      << std::endl;
        else // Error while reading
            std::cout << "Error while reading response!\n";

    } while(true); // End user input loop

    client.Close(); // Close socket
    return 0;
}
