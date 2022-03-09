''' Plot results from the experiments in this directory.
    author: Daniel Nichols
    date: February 2022
'''
# std lib
import sys
from os import makedirs
from os.path import join as path_join

# tpl 
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker



def make_rank_plots(df):
    ''' Generates plots for the rank experiment. (1) N vs Rank Time (2) N vs Overhead
        df columns: problem,bitvector_size,num_rank_calls,num_iter,overhead,avg_duration
    '''
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(8,5))
    times = df['avg_duration']
    ax1.plot(df['bitvector_size'], times)
    ax1.set_xscale('log')
    ax1.set_ylabel('Duration to Call Rank 1e5 Times (seconds)')
    range = times.max() - times.min()
    ax1.set_ylim(times.min()-0.5*range, times.max()+0.5*range)
    ax1.yaxis.set_major_formatter(ticker.FormatStrFormatter('%.1e'))

    ax2.plot(df['bitvector_size'], df['overhead'])
    ax2.set_xscale('log')
    ax2.yaxis.tick_right()
    ax2.set_ylabel('# Bits Overhead')
    ax2.yaxis.set_major_formatter(ticker.FormatStrFormatter('%.0e'))
    
    fig.text(0.5, 0.02, 'BitVector Size (N)', ha='center')
    fig.suptitle('Rank Duration and Overhead vs BitVector Size (N)')

    fig.savefig(path_join('figs', 'rank-experiment-plots.png'))


def make_select_plots(df):
    ''' Generates plots for the select experiment. (1) N vs select Time
        problem,bitvector_size,num_select_calls,num_iter,overhead,avg_duration
    '''
    
    fig, ax = plt.subplots(1, figsize=(8,5))
    times = df['avg_duration']
    ax.plot(df['bitvector_size'], times)
    ax.set_xscale('log')
    ax.set_ylabel('Duration to Call Select 1e5 Times (seconds)')
    range = times.max() - times.min()
    ax.set_ylim(times.min()-0.5*range, times.max()+0.5*range)
    ax.yaxis.set_major_formatter(ticker.FormatStrFormatter('%.1e'))
    
    ax.set_xlabel('BitVector Size (N)')
    fig.suptitle('Select Duration vs BitVector Size (N)')

    fig.savefig(path_join('figs', 'select-experiment-plots.png'))


def make_sparsearray_plots(df):
    ''' Generates plots for the sparsearray experiment. (1) N vs Append Time (2) N vs GetAtIndex (3) N vs GetAtRank
                                                        (4) N vs Overhead
        df columns: problem,array_size,sparsity,num_func_calls,dense_overhead,sparse_overhead,avg_append_duration,
                    avg_getatindex_duration,avg_getatrank_duration
    '''
    
    #plt.clf()
    fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(12,5))
    
    df.reset_index().pivot('array_size', 'sparsity', 'avg_append_duration').plot(ax=ax1, title='Append', xlabel='', ylabel='Seconds', logx=True)

    df.pivot('array_size', 'sparsity', 'avg_getatindex_duration').plot(ax=ax2, title='GetAtIndex', xlabel='', legend=False, logx=True)

    df.pivot('array_size', 'sparsity', 'avg_getatrank_duration').plot(ax=ax3, title='GetAtRank', xlabel='', legend=False, logx=True)

    fig.text(0.5, 0.01, 'Array Size (N)', ha='center')
    fig.suptitle('Time in Functions vs Array Size')
    fig.tight_layout()
    fig.savefig(path_join('figs', 'sparsearray-experiment-plots.png'))


    fig, ax = plt.subplots(1, figsize=(8,5))
    df.pivot('array_size', 'sparsity', 'sparse_overhead').plot(ax=ax, title='Sparse Memory Savings', xlabel='Array Size (N)', logx=True)
    ax.plot(df[df['sparsity'] == 0.1]['array_size'], df[df['sparsity'] == 0.1]['dense_overhead'], color='black', linestyle='dashed', label='1.0')
    ax.legend()
    ax.set_ylabel('# Bits')
    fig.tight_layout()
    fig.savefig(path_join('figs', 'sparsearray-overhead-experiment-plots.png'))


def main():
    if len(sys.argv) != 4:
        print('usage: {} <rank_csv> <select_csv> <sparsearray_csv>'.format(sys.argv[0]), sys.stderr)
        exit(1)

    # mkdir -p figs
    makedirs('figs', exist_ok=True)

    # read in data
    rank_df = pd.read_csv(sys.argv[1])
    select_df = pd.read_csv(sys.argv[2])
    sparsearray_df = pd.read_csv(sys.argv[3])

    # make plots
    make_rank_plots(rank_df)
    make_select_plots(select_df)
    make_sparsearray_plots(sparsearray_df)



if __name__ == '__main__':
    main()