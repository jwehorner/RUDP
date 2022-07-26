# **RUDP**

## **Contents**

* [About](#about)
* [Prerequisites](#prerequisites)
* [Testing](#test)
* [Contact](#contact)

## **About**

A library for reliably sending UDP packets between two Connection objects using the Stop-and-Wait ARQ protocol. The 
library can be used directly as C++ or compiled into a .lib/.a then used in C through the rudp.h header.

## **Prerequisites**

* CMake version 3.22
* Boost version 1.75

## **Test**

Build and run test_connection.cpp or test_send.c and test_recv.c using the provided CMakeList

## **Protocol Implementation**
A number of additions have been made to the implemented ARQ protocol to facilitate use of the library in certain 
applications. 

### **Connection Implementation**
The first minor difference is that one connection object can send to only one other connection object, 
however it can receive from multiple connection objects. The connection has a sequence number for sending and n sequence 
numbers for each connection that has been received from.

#### **Stop-and-Wait ARQ Implmentation**
The protocol was implemented with slightly different behaviour depending on the relationship between the senders' sequence 
number and the receivers' sequence number:

#### **Sender Seq < Receiver Seq**
If the receiver receives a packet with a sequence number less than its own, an ACK will be sent back but the receive() 
function will not return as it is assumed that the packet was received previously but the ACK did not get delivered to 
the sender, so the receiving application already has the data that was sent.
```
Sender		Receiver
1	|\ 1	| 1
	| \		|
	|  \	|
	|	\	|
 	|	 \	|
	|	  \	|
	|	   \| 
	|	 1 /| 2
	|	  /	|
	|	 /	|
	|	X	|
1	|\ 1	| 2
	| \		|
	|  \	|
	|	\	|
 	|	 \	|
	|	  \	|
	|	   \| 
	|	 1 /| 2
	|	  /	|
	|	 /	|
	|	/	|
	|  /	|
	| /		|
2	|/		| 2
```
#### **Sender Seq == Receiver Seq**
If the receiver receives a packet with a sequence number equal to its own, an ACK will be sent back and the receive() 
function will return.
```
Sender		Receiver
1	|\ 1	| 1
	| \		|
	|  \	|
	|	\	|
 	|	 \	|
	|	  \	|
	|	   \| 
1	|	 1 /| 2
	|	  /	|
	|	 /	|
	|	/	|
	|  /	|
	| /		|
2	|/		| 2
```
#### **Sender Seq > Receiver Seq**
If the receiver receives a packet with a sequence number greater than its own, an ACK will be sent back and the receive() 
function will return as it is assumed that at some point the receiver has been reset so it must catch up to the sender.
```
Sender		Receiver
3	|\ 3	| 0
	| \		|
	|  \	|
	|	\	|
 	|	 \	|
	|	  \	|
	|	   \| 
3	|	 3 /| 3
	|	  /	|
	|	 /	|
	|	/	|
	|  /	|
	| /		|
3	|/		| 3
```

## Contact

James Horner - JamesHorner@cmail.carleton.ca or jwehorner@gmail.com
