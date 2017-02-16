sws:
	gcc -o sws sws.c -pthread -Wall

clean:
	rm -rf sws
	rm -rf udp_client

run:
	./sws 8080 www

runthread:
	./sws 8080 www --pthread

client:
	gcc -o udp_client udp_client.c

runclient:
	./udp_client
