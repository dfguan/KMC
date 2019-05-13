/*
 * =====================================================================================
 *
 *       Filename:  kmer_distri.cpp
 *
 *    Description:  find kmer distribution 
 *
 *        Version:  1.0
 *        Created:  13/05/2019 14:39:17
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dengfeng Guan (D. Guan), dfguan@hit.edu.cn
 *   Organization:  Center for Bioinformatics, Harbin Institute of Technology
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <zlib.h>
#include <vector>
#include "kseq.h"
#include "kmc_file.h"


KSEQ_INIT(gzFile, gzread, gzseek)


int print_kmd(std::vector<uint32_t> &counter, char *sn)
{
	uint32_t s_counter = counter.size();
	for ( uint32_t i = 0, j = 1; j <= s_counter; ++j) {
		if (j == s_counter || counter[i] != counter[j]) {
			fprintf(stdout, "%s\t%u\t%u\t%u\n", sn, i, j, counter[i]);	
			i = j;	
		}
	}
	return 0;
}

int kmer_distri(char *fn, char *kmerdb)
{
		
		
	gzFile fp = fn && strcmp(fn, "-") ? gzopen(fn, "r") : gzdopen(fileno(stdin), "r");
	if (fp == 0) return 1;
	kseq_t *seq = kseq_init(fp);

	CKMCFile kdb; 
	if (!kdb.OpenForRA(kmerdb)) return 1;
	std::vector<uint32_t> cnts;
	while (kseq_read(seq) >= 0) {
		kdb.GetCountersForRead(seq->seq.s, cnts);
		for (uint32_t i =0, j = 1; j <= cnts.size(); ++j) {
		
		
		
		}
	}
	return 0;
}


int main(int argc, char *argv[])
{

}

