import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
import sys

#from kat tools script
def findpeaks(t):
    b = np.squeeze(np.asarray(t))
    d = np.sign(np.diff(b))
    d[d == 0] = 1
    return np.where(np.diff(d) == -2)[0] + 1

def spectra_plot(input_fl, out_fl, title):
    title = title 
    xlabel = "k-mer multiplicity"
    ylabel = "Number of distinct k-mers"
    height = 6
    width = 8
    dpi = 300
    colors = ["#000000", "#ef2928", "#ad7fa8", "#8ae234", "#729fcf", "#f2c27e", "#fcaf3e", "#fce94f"]
    input_fp = open(input_fl)
    mat = np.loadtxt(input_fp)
    input_fp.close()
    
    #adjust matrix and set read-depth cutoff
    mat = np.delete(np.transpose(mat), (0), axis = 0)
    nrow = mat.shape[0]
    ncol = mat.shape[1]
    
    totals = mat.sum(axis = 0)
    ymin = 0
    peakx = findpeaks(totals)
    peakx = peakx[peakx != 1]
    peaky = totals[peakx]
    ymax = np.max(peaky)
    ylim = ymax * 1.5
    xmin = 0
    # from back to end find ysum > 0.001 * ymax 
    exclude_last_col = True
    if exclude_last_col:
        for i, e in reversed(list(enumerate(totals[:-1]))):
            if e / ymax > 0.01: 
                xlim = i
                break
    else:
        for i, e in reversed(list(enumerate(totals))):
            if e / ymax > 0.01: 
                xlim = i
                break
    mat = mat[:,:xlim] 
    nrow = mat.shape[0]
    ncol = mat.shape[1]
    # print (mat)
    bottoms = [[0] * ncol]
    for i in range(nrow - 1):
        bottoms.append(np.add(bottoms[i], mat[i,:].tolist()).tolist())
    plt.figure(num=None, figsize=(width, height))
    # print (bottoms)
    plt.axis([0, xlim, 0, ylim])
    x = range(ncol)
    labs = [ "{}X".format(e) for e in range(nrow-1)]
    labs.append("{}X+".format(nrow-1)) 
    
    for i in range(nrow):
        bar = plt.bar(x, mat[i,:].tolist(), bottom = bottoms[i], edgecolor=colors[i], color = colors[i], width = 1, label = labs[i])
        # bar = plt.bar(x, mat[i,:].tolist(), bottom = bottoms[i], edgecolor=colors[i], color = colors[i],label = labs[i])
    
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True, color="black", alpha=0.2)
    plt.legend(loc = 1)
    plt.tight_layout()

    plt.savefig(output_file, dpi = dpi)


if __name__ == "__main__":
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    if len(sys.argv) > 3:
        title = sys.argv[3]
    else:
        title = "k-mer comparison plot"
    spectra_plot(input_file, output_file, title)
