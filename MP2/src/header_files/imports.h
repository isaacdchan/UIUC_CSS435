#pragma once

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <bits/stdc++.h>

#define MAX_RESIDENTS 256
#define EXPIRATION_THRESHOLD 600
#define VERBOSE true

#define nodeID_size sizeof(short int)
#define ttl_size sizeof(short int)
#define op_size 4