btsoot:
	clang -O2 -std=c11 btsoot.c backup.c db.c copy.c xxhash.c sqlite3.c -o btsoot.x86_64 -pthread -ldl
test:	sqlite3.o btsoot.o xxhash.o db.o backup.o copy.o
	clang -Wall -Wextra -O2 -march=native -frename-registers -std=c11 btsoot.o backup.o db.o copy.o xxhash.o sqlite3.o -o btsoot.x86_64 -pthread -ldl
production:
	clang-5.0 -O2 -std=c11 btsoot.c backup.c db.c copy.c xxhash.c sqlite3.c -o btsoot.x86_64 -pthread -ldl
install:
	cp btsoot.x86_64 /usr/local/bin/btsoot
	mkdir -p /etc/btsoot/scans
update:
	cp btsoot.x86_64 /usr/local/bin/btsoot
remove:
	rm /usr/local/bin/btsoot
	rm -rf /etc/btsoot
clean:
	rm btsoot *.o
