import numpy as np


import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
import sys, argparse

#from kat tools script
def findpeaks(t):
    b = np.squeeze(np.asarray(t))
    d = np.sign(np.diff(b))
    d[d == 0] = 1
    return np.where(np.diff(d) == -2)[0] + 1

def spectra_plot(ycut, xcut, yflim, xflim, input_fl, out_fl, prefix, isstk):
    title = "stacked " if isstk else "" 
    title += prefix + " k-mer comparison plot" 
    # title2 = prefix + " k-mer comparison plot"
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
    ylim = ymax * ycut  if yflim == -1 else yflim
    xmin = 0
    # from back to end find ysum > 0.001 * ymax 
    xlim = xflim 
    if xlim == -1:
        exclude_last_col = True
        if exclude_last_col:
            for i, e in reversed(list(enumerate(totals[:-1]))):
                if e / ymax > xcut: 
                    xlim = i
                    break
        else:
            for i, e in reversed(list(enumerate(totals))):
                if e / ymax > xcut: 
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
    x = range(ncol)
    labs = [ "{}X".format(e) for e in range(nrow-1)]
    labs.append("{}X+".format(nrow-1)) 
    # plt.subplot(2,1,1) 
    plt.axis([0, xlim, 0, ylim])
    if isstk:
        for i in range(nrow):
            bar = plt.bar(x, mat[i,:].tolist(), bottom = bottoms[i], edgecolor=colors[i], color = colors[i], width = 1, label = labs[i])
            # bar = plt.bar(x, mat[i,:].tolist(), bottom = bottoms[i], edgecolor=colors[i], color = colors[i],label = labs[i])
    else:
        for i in range(nrow):
            plt.plot(x, mat[i,:].tolist(), label = labs[i], color=colors[i])

    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True, color="black", alpha=0.2)
    plt.legend(loc = 1)

    # plt.subplot(2,1,2) 
    # plt.axis([0, xlim, 0, ylim])
    # for i in range(nrow):
        # bar = plt.bar(x, mat[i,:].tolist(), edgecolor=colors[i], color = 'None', width = 1, label = labs[i])
    # plt.title(title2)
    # plt.xlabel(xlabel)
    # plt.ylabel(ylabel)
    # plt.grid(True, color="black", alpha=0.2)
    # plt.legend(loc = 1)

    plt.tight_layout()

    plt.savefig(out_fl, dpi = dpi)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Genome Comparison plot')

    parser.add_argument('-y', '--ycut', type=float, action="store", dest = "ycut", help ='set y axis limit to N times of ymax [1.5]', default = 1.5)
    parser.add_argument('-x', '--xcut', type=float, action = "store", dest = "xcut", help = 'set x axis limit to where N times of ymax is [0.01]', default = 0.01)
    parser.add_argument('-Y', '--ymax', type=int, action = "store", dest = "ylim", help = 'set y axis limit [-1]', default = -1)
    parser.add_argument('-X', '--xmax', type=int, action = "store", dest = "xlim", help = 'set x axis limit [-1]', default = -1)
    parser.add_argument('-t', '--title', type = str, action = "store", dest = "title", help = 'figure title [NULL]', default="")
    parser.add_argument('-s', '--stacked', action='store_true', dest="isstk", default=False)
    parser.add_argument('-v', '--version', action='version', version='spectra 0.0.0')
    parser.add_argument('mat_fn', type=str, action="store", help = "matrix file")
    parser.add_argument('png_fn', type=str, action="store", help = "output png file")
    opts = parser.parse_args()
    spectra_plot(opts.ycut, opts.xcut, opts.ylim, opts.xlim, opts.mat_fn, opts.png_fn, opts.title, opts.isstk)
