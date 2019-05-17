# KMC (modified by dfguan)
This is a modified [KMC](https://github.com/refresh-bio/KMC) to compare K-mers from short reads data and an assembly, and make a plot.

# Installation

```
git clone https://github.com/dfguan/KMC.git && cd KMC
make -j 16 
```
if you compile the source code sucessfully, there will be a bin directory including all exectuable files you need. Otherwise, please refer to the old [README](https://github.com/dfguan/KMC/blob/master/README.rb.md). 

# Quick Start

Given an assembly and short read file list, you can use the following commands to make a comparison plot.

```
bin/kmc -ci0 -fm -t12 $asm $asm.prefix tmp
bin/kmc -ci0 -t12 -m20 $reads $reads.prefix tmp
bin/kmc_tools analyze $reads.prefix $asm.prefix $output.matrix
python3 spectra.py $output.matrix $output.png
```
when all the commands are finished, you will see a figure like this: ![kmc_plot.png]()


# Instruction

1. How to make a read file list  
	the read file list is a <tab> deliminated text file, each read file a line, following a simple syntax: \<READ\_FILE_PATH\>\<tab\>\[TRIM\_NUMBER\]. Please notice if the trim\_number is not set, it's treated as 0. If you only have one read file, you can use kmc without a read file list, and you can use ``-d`` to set trimmed off bases.
	
	
# Notice:
This plot is just a small part learned from a K-mer Analysis Toolkit [(KAT)](https://github.com/TGAC/KAT). If you'd like to know more, please go to their website: [kat-web](http://www.earlham.ac.uk/kat-tools).