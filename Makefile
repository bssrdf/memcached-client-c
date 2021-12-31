#
# Makefile for Chapter 01
#
# Type  make    to compile all the programs
# in the chapter 
#
all: memcache

clean:
	rm -f memcache

memcache: memcached_client.c
	cc -o memcache memcached_client.c 



