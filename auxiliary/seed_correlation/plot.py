import numpy, matplotlib.pyplot, mplcyberpunk

# --- Import data ---
# -------------------

directory = 'temp/seed_correlation/'

data_minstd_rand = numpy.genfromtxt(directory + 'minstd_rand.csv', delimiter=',')
data_mt19937     = numpy.genfromtxt(directory +     'mt19937.csv', delimiter=',')

# --- Plot ---
# ------------

matplotlib.pyplot.style.use('cyberpunk')

def plot_data(data, title, filename):
    figure = matplotlib.pyplot.figure(dpi=800)

    axes = figure.add_subplot()

    axes.imshow(data, cmap='binary', interpolation='nearest')

    axes.grid(False)
    axes.set_xticks(numpy.linspace(0, 200, 6))
    axes.set_yticks(numpy.linspace(0, 160, 5))
    axes.set_title(title, y=1.04)

    figure.savefig(directory + filename)
    
plot_data(data_minstd_rand, 'std::minstd_rand', 'random_seed_correlation_minstd_rand.svg') # visually noticeable correlation
plot_data(data_mt19937    , 'std::mt19937'    , 'random_seed_correlation_mt19937.svg'    ) # no significant correlation