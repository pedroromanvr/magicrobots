Introduction

This page includes both the code and specification for a nRF24L01 based P2P protocol and utilities.

The code was designed to work with both Code Vision and AVR Studio compilers. (Code Vision version has been tested extensively)

The nrf24l01 is an ultra-low power RF transceiver IC for the 2.4GHz ISM (Industrial, Scientific and Medical) band. This document describes a possible solution for embedded systems to use this module to create a real-mesh wireless network for all kind of communication purposes. Also, as an appliance to this, a cooperative robotic system is proposed.

Details

Main characteristics:

Auto-configurable mesh with two main roles: * Root node. Provides interconnection services to all its leaf nodes * Leaf node. Communicates with other nodes through its paired root node

Nearest neighbor detection

Node address is selected randomly in a range

All nodes can be either root or leaf

Leafs ask petitions to roots for connection

Root auto-connect based on address (16 bit)

Interfaces

The interfaces created for the network stack are:

/* ------ To use outside this API ------ */ 
// Root: Saves leaf nodes in our RIP table. 
// Leaf: Tries to locate the nearest root node. 
ret_t joinNetworkOnTheFly(discPack_p pp);

// Forward a broadcast message, used by root
ret_t forwardMessage(headerPack_p header, char *msg, uint8_t size);

// Broadcast message
// Send message to all nodes in the RIP table 
ret_t sendMessage(char msg, uint8_t size);

/ * Advanced way to send a message. 
  * No verification of any type is done. 
  * header + msg are transmited in one packet. 
  */ 
  ret_t microSendMessage(headerPack_p header, char *msg, uint8_t size); 
//Send message to an specific ID
ret_t sendMessageTo(headerPack_p header, char msg, uint8_t size); 

/ * header is used to retrieve information 
  * about the message. 
  * In getMessageFrom it is also used to indicate 
  * the source ID */ 
  //Recieve message from any sender 
  ret_t getMessage(headerPack_p header, char *buf, uint8_t size);

//Specify ID to recieve from 
ret_t getMessageFrom(headerPack_p header, char *buf, uint8_t size); 
