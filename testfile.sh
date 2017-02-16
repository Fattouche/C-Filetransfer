## required test cases (found in the www folder)
#run with ./sws 8080 www
echo -e -n "GET / HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 & ##200 OK
echo -e -n "GET / HTTP/1.1\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 & ## 400 Bad Request
echo -e -n "GET /nofile HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 & ## 404 Not found

## 5 extra test cases comment these out to see the other cases need to use the tests folder and not the www folder
#run server with ./sws 8080 tests
echo -e -n "GET /gnu/main.html HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 & #large file -- 200 OK
echo -e -n "GET /../test.html HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 & #bad request -- 404 Not Found
echo -e -n "got /gnu/main.html http/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 & #got instead of GET -- 400 Bad Request
echo -e -n "GET index.html HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 & # no initial slash -- 400 Bad Request
echo -e -n "GET /gnu/mainer.html HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 & #Incorect file -- 404 Not Found

wait
