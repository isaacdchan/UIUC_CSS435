In this task, you will implement a simple HTTP client. The client should be able to GET files correctly from standard web servers. 

HTTP Communication Basics
1. The client sends an HTTP GET request to the server.
2. Server replies with a response based on client request
3. Client takes further action based on response


Sample Request
```
GET /test.txt HTTP/1.0
User-Agent: Wget/1.15 (linux-gnu)
Accept: */*
Host: localhost:3490
Connection: Keep-Alive
```

Sample Response
```
HTTP/1.1 200 OK
Date: Sun, 10 Oct 2010 23:26:07 GMT
Server: Apache/2.2.8 (Ubuntu) mod_ssl/2.2.8 OpenSSL/0.9.8g
Last-Modified: Sun, 26 Sep 2010 22:04:35 GMT
ETag: "45b6-834-49130cc1182c0"
Accept-Ranges: bytes
Content-Length: 13
Connection: close
Content-Type: text/html
```

Per the HTTP standard, again an empty line (two CRLF, “\r\n\r\n”) marks the end of the header, and the start of message body.

Common response headers:

`HTTP/1.0 200 OK` - returning the requested document

`HTTP/1.0 404 Not Found` - requested file does not exist

`HTTP/1.0 301 Moved Permanently` - server was moved to a different address


1. Parse user request
   1. If protocol is not HTTP, write "INVALIDPROTOCOL" to output
   2. ex: `./http_client htpt://localhost/file`
2. If the client can't connect to the target host, write "NOCONNECTION" to output 
3. Send an HTTP GET request based on the first argument it receives. 
4. If 404 (non-existent) response, write "FILENOTFOUND" to output
5. Write the message body of the response to output
   1. you need to be able to receive different file types
   2. file size may be up to hundreds of MBs.