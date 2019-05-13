CC      =  g++
CFLAGS  =  -g -Wall  -D DEBUG #-O2 
LDFLAGS = -lz -lm

PROG = kmd 

.SUFFIXS:.cpp .o

all:$(PROG)

kmd: kmc_file.o kmer_distri.o  kmer_api.o mmer.o 
	$(CC)  $(CFLAGS) $^ -o $@ $(LDFLAGS) 

.cpp .o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(PROG)

	
