# fips202.o: fips202.h fips202.c
# 	gcc -c -std=c11 fips202.c 

# rng.o: rng.h rng.c
# 	gcc -c -std=c11 rng.c -I/usr/local/opt/openssl/include 

# main.o: fips202.h rng.h main.c
# 	gcc -c -std=c11 main.c -I/usr/local/opt/openssl/include 


# test: fips202.o rng.o main.o
# 	gcc -o test *.o -L/usr/local/opt/openssl/lib -lcrypto

simple:
	gcc  -Wall -Wextra -march=native -mtune=native -O3 -fomit-frame-pointer  -c *.c -I/usr/local/opt/openssl/include && gcc -o farl *.o -L/usr/local/opt/openssl/lib -lcrypto

clean:
	rm -f *.o test