# DNS-Resolver

This assignment was completed as part of my Operating Systems class. The goal was to use pthreads to code a DNS name resolution engine. This is a multithreaded application written in C that resolvers domain names to IP addresses, which essentially boils down to the bounded buffer problem. There are two subsystems, a requester pool and a resolver pool, that will communicate with each other using a shared array. Util.h and util.h were provided and contain the DNS lookup function, which accepts a hostname and generates an IP string as output. The program main.c processes one name file that contains the name of servers and IP addresses. Each requester thread will read each line of the file, parse the hostname, place that hostname into a shared data array, and record that processing in serviced.txt. The resolver threads take the hostnames from the shared array, match it to an IP address and write the results to results.txt. This assignment required knowledge of critical sections, deadlock, pthread condition variables, and mutexes.

To run, use main (number of requesters) (number of resolvers) serviced.txt results.txt (name of one namefile)

