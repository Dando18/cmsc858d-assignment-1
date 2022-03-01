''' Plot results from the experiments in this directory.
    author: Daniel Nichols
    date: February 2022
'''
import sys

import pandas as pd
import matplotlib.pyplot as plt


def make_rank_plots(df):
    ''' Generates plots for the rank experiment. (1) N vs Rank Time (2) N vs Overhead
    '''
    plt.clf()
    plt.plot(df['bitvector_size'], df['avg_duration'])
    plt.title('BitVector Size (N) vs Time to Call Rank 1E5 Times')
    plt.show()

def main():
    if len(sys.argv) != 4:
        print('usage: {} <rank_csv> <select_csv> <sparsearray_csv>'.format(sys.argv[0]), sys.stderr)
        exit(1)

    # read in data
    rank_df = pd.read_csv(sys.argv[1])
    select_df = pd.read_csv(sys.argv[2])
    sparsearray_df = pd.read_csv(sys.argv[3])

    # make plots
    make_rank_plots(rank_df)



if __name__ == '__main__':
    main()