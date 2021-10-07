# Background
Goal: Implement shortest-path routing. Use link state, distance vector, or path vector protocol

## iptables
Application that allows configuration of rules that will be enforced by the kernel's `netfilter` framework

# Routing Environment
Actions - drop, tag, allow
Based on - dest address/port, protocol

## Nodes
Each node in the network will have

* ID - unique identifier from 0-255
* Virtual Interface - `eth0:1`, `eth0:2`
* Address - 10.1.1.<ID>

## Rules
Only neighboring nodes/addresses can communicate

## Edges
Nodes do not know complete toplogy. Only know direct neighbors

# Manager
Sends 2 types of messages to nodes on UDP port 7777
* Instructions - send a data packet into the network
* Topology Updates - edge to node X now has cost N

IMPORTANT: Must convert from `network order` to `host order`

Packet
```
# Format
"send" (4 ASCII bytes)
destID (net order 2 bytes)
message (ASCII message < 100 bytes)

# Example
"send4hello"
```

Update
```
# Format
"cost" (4 ASCII bytes)
destID (net order 2 bytes)
newCost (net order 4 bytes)

# Example
"cost533"
```

Include function that sets link cost to -1 to signify no cost

# Router
```
# run command format + example
<binary file> <nodeid> <initial costs file> <log file output name>
./vec_router 0 costs0.txt test3node0.log
```

## Logging
Create log file by default

Node should log all packet recvs, sends, forwards even if the dest is unreachable

Must monitor whether an edge is live *then* check cost in costs file

## Responsibilities
1. Initialize forwarding + route table
   1. For LS, may have to perform initial mapping algo
2. Forward packets according to forwarding table
3. React to downed edges. Update routes + forwarding tables

## Edge Cases
Network partitions: node(s) that have no routes to reach them

Tie breaking: When two routes have the same cost, length, etc.
* Choose the path whose next hop router has the lowest ID


# File Formats
## Initial Costs
```
<neighboring node ID> <cost>
200 235555
```
* Missing node neighbor entries have default cost 1
* Only *if* an edge is live between node X and neighbor Y will the cost be N

**NEVER 0 OR 1 COST CONNECTIONS**

## Log File
```bash
### FORMAT
op | <src node> | <dest node> | <next hop node> | <content>
route | <src node> | <dest node> | <cost> | <comma-separated-path>
send
cost
```

```c
char* logline;
sprintf(logLine, "send | %d | %d | %s\n", dest, nexthop, message); 
fwrite(logLine, 1, strlen(logLine), theLogFile)
```

flush the buffer to ensure the output is written to stable storage

# Extra notes
