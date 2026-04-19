YUSUF BILGICH
11848141

-> How to compile and run your program
You can make with:
make
Then you can run with:
./PA5-finalCode 8080
In a second terminal, you can send yout HTTP GET requests with curl (an example:
curl -x localhost:8080 http://example.com/
)

-> Assumptions made, if any
The proxy is a single-threaded, iterative implementation
The proxy is only designed to handle HTTP GET requests

-> Example input and output
Terminal 1:
./PA5-finalCode 8080
Proxy listening on port 8080

Terminal 2:
curl -x localhost:8080
 http://example.com
<!doctype html><html lang="en"><head><title>Example Domain</title><meta name="viewport" content="width=device-width, initial-scale=1"><style>body{background:#eee;width:60vw;margin:15vh auto;font-family:system-ui,sans-serif}h1{font-size:1.5em}div{opacity:0.8}a:link,a:visited{color:#348}</style></head><body><div><h1>Example Domain</h1><p>This domain is for use in documentation examples without needing permission. Avoid use in operations.</p><p><a href="https://iana.org/domains/example">Learn more</a></p></div></body></html>