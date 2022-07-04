QtChatTCP
===
A simple client-server TCP architecture to chat between peers.

The architecture is made up of 3 parts:

* Client
* Server
* TransferProtocol

You can instantiate as many Clients as you wish.

Features
---

1. Broadcast transferring
2. Single channel transferring
3. Updated to latest slot-signal syntax found in Qt 5.15
4. It's possible to message any online person

Protocol
---
The protocol I've designed is quite simple. It sends 4 bytes (lets call them C) and JSON with size of C bytes and
following fields:

1. type. This is an enum element
2. from (for messages)
3. to (for messages)
4. descriptor (for connect/disconnect/sending descriptor). This is a list of descriptors
