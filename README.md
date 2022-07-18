# RUDP

## Contents

* [About](#about)
* [Prerequisites](#prerequisites)
* [Testing](#test)
* [Contact](#contact)

## About

A library for reliably sending UDP packets between two Connection objects using the Stop-and-Wait ARQ protocol. The 
library can be used directly as C++ or compiled into a .lib/.a then used in C through the rudp.h header.

## Prerequisites

* CMake version 3.22
* Boost version 1.75

## Test

Build and run test_connection.cpp or test_send.c and test_recv.c using the provided CMakeList

## Contact

James Horner - JamesHorner@cmail.carleton.ca or jwehorner@gmail.com
