MiniCord: A Light-Weight Chat System

Project Overview and Goals
MiniCord is a light-weight, multi-client chat system tailored for business environments. It utilizes event-driven I/O to manage concurrent network connections efficiently without the heavy overhead of multi-threading on the backend. The architecture centers around a centralized server written in C and lightweight graphical clients written in Python (Tkinter based GUI client).
The system implements a custom application-layer protocol to support core chat operations:
Client Authentication: Users can log in using unique usernames.
Channel Creation & Subscription: Users can join up to 10 distinct channels, seamlessly switching between them.
Direct Messaging (Unicast): Users can click on active peers to spawn temporary DM tabs for private communication.
Channel Broadcasting (Multicast): Messages are cleanly routed to all authenticated users within the same active channel.
Presence Notifications: The server tracks and broadcasts online, offline, and channel-switching events to keep client interfaces perfectly synchronized.

Themes Used
This project directly applies the following four core systems programming themes:
1. Event-Based Concurrency Instead of dedicating a separate thread to each client connection (which can exhaust memory and CPU overhead), the backend server uses a single-threaded, non-blocking architecture. By utilizing the poll() multiplexing system call, the server monitors an array of file descriptors, dynamically reacting to POLLIN events when data is ready to be read.
2. Network Programming The core communication relies on TCP sockets. The server initializes a master listening socket bound to a specific port (9091) and accepts incoming client connections. Developed a custom application-layer messaging protocol where the server enforces a standardized "[Server]:" prefix format for system alerts, allowing the lightweight clients to easily parse using my standardized communication protocol.
3. System-Level I/O The server strictly uses system-level I/O (read() and write()) rather than standard I/O (printf/scanf) for network transmission to avoid internal buffering corruption. Sockets are configured as non-blocking using fcntl and the O_NONBLOCK flag. The system efficiently manages the File Descriptor Table (FDT) by identifying when a read() returns 0 (EOF), gracefully freeing the resource, and actively packing the poll() array to avoid open-socket leaks.
4. Control Flow The project relies on intricate system-level control flow to manage the asynchronous nature of the application. On the backend, the poll() loop intercepts the standard sequential execution, only processing users who have actively triggered an event. On the frontend, control flow is split across threads—a background network loop intercepts raw byte streams, parses state changes, and triggers UI updates, while the main thread handles user input.

Design Decisions and Trade-offs
Centralized Server vs. Lightweight Clients: Unlike how it may seem at a first glance, the clients do not actually hold any data, only the C server does. The Python clients hold "soft state" (like who is in a channel) but rely entirely on the server's broadcast alerts (which are standardized and start with [Server]:) to update their graphical user lists. This keeps the client codebase simple and prevents state desynchronization.
Single-Threaded Server vs. Multi-Threaded Client: The server uses a single-threaded architecture with the help of poll(), while the client side uses a multi-threaded architecture to seperate network package handling and UI rendering processes. The client dedicates one thread to Tkinter's mainloop() for UI rendering and a separate background thread for blocking recv() calls to ensure the GUI does not freeze when waiting for network packets.
Strict String Parsing Constraints: To maintain safe buffer sizes and prevent overflows in C, I utilized sscanf with strict format specifiers (like %31s). The trade-off here is that multi-word usernames or channels with spaces would be truncated at the first space, inherently using a single-word naming convention.

Challenges Encountered and Lessons Learned
Recycled File Descriptors & Phantom Reads: Early in development, there was a bug that kicked any new clients attempting to joing the server. After some research, I learned that when the OS recycles a recently closed file descriptor, the poll() array can retain a leftover POLLIN flag in the revents field. Because the socket was non-blocking, the server attempted a phantom read(), received an EAGAIN error, and treated it as a fatal disconnect. We solved this by explicitly clearing fds[nfds].revents = 0 upon connection and properly checking for EAGAIN/EWOULDBLOCK conditions.
Client-Server User State Synchronization: The "Users" list on the right side was the biggest hurdle in development for this program. In my initial stages of development, the "user has come online" call did not include a specific server, so anytime a user would login, a universal message would be broadcast that the user has logged in (without specifying which server they were in). This resulted in a confusion on the client side and they were automatically added to the user list in any server the client might be in. This made me change the "turned online" broadcast to be more specific and also include the server the user is now in. Also, the existence of so many conditions when switching channels (going from X to Y channel, going offline in Y channel, switching back to another channel, etc.) was tedious to write up the user list synchornization commands for all those conditions.

Setup and Installation

Prerequisites
Backend Server: A Linux/Unix environment with make installed.
Client GUI: Python 3.x installed

Compilation and Running
The server must be started first before any clients can connect. Open a terminal, navigate to the project directory.
1. Compile the server using make:
make
2. Then run the server with:
./minicord_server
The server will start listening for incoming TCP connections on port 9091.
3. Run the Python Client
Open a new terminal window and run the client script:
python3 minicord_client.py
4. Usage Instructions
When the GUI launches, enter a single-word username and click Login.
You will automatically join the #General channel.
Click "+ Join Channel" to create or join a new channel.
Click on any user's name in the right-hand Users list to open a private Direct Messaging (DM) tab.