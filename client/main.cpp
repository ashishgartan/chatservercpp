#include "ChatClient.h"

int main()
{
    ChatClient client;
    return 0;
}
/*
.
├── main.cpp              // 🔁 Entry point: creates and runs ChatClient
├── ChatClient.h          // 📘 Declaration of ChatClient class
├── ChatClient.cpp        // 🔧 Implements ChatClient (connect, login, messaging, etc.)
├── ImageHandler.h        // 📘 Declarations for sendImage() and receiveImage()
├── ImageHandler.cpp      // 🧰 Contains logic for sending/receiving images
├── Makefile              // 🧪 Builds all files into one executable (chatclient)

*/