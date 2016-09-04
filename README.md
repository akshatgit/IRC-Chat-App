A c++ implementation for the IRC Chat App. 
IRC. The program would involve a central chat portal, a server listening on one port (let’s say ports X). The program supports multiple users communicating through the portal. The clients use their client version to connect to the portal and communicate to other users. Before communicating, the users need to register to the chat portal. To achieve this they clients connect to port X, the registration service, and register their username and password. Once registered, the username and password are stored in plaintext in a local file at the server. Thereafter, the client connects to the chat server (On the same port) which authenticates users based on username and password, previously registered. Once authenticated the user sees a portal prompt wherein he can type commands to chat with other users, transfer files etc. The following commands are supported

“/who” : Who all are logged in.

“/msg”: Send message to particular user.

“/create_grp”: Create a group

“/join_grp”: Join a group

“/send” : Send file to another user.

“/msg_group” : Broadcast message to a particular group once you have joined it

“/recv” : Receive the file that is being sent by someone.

NOTE: Communication between peers is via the chat portal and not through direct p2p communication.
