import numpy, matplotlib.pyplot, mplcyberpunk

# --- Import data ---
# -------------------

directory = 'temp/approx_distributions/'

data_approx  = numpy.genfromtxt(directory +  'approx_normal.csv', delimiter=',')
data_precise = numpy.genfromtxt(directory + 'precise_normal.csv', delimiter=',')

# --- Analytical PDF ---
# ----------------------

x = numpy.linspace(-4, 4, 100)
y = 1.0 / numpy.sqrt(2 * numpy.pi) * numpy.exp(-0.5 * x * x) # normal distribution PDF, mean = 0, variance = 1

# --- Plot ---
# ------------

matplotlib.pyplot.style.use('cyberpunk')

figure = matplotlib.pyplot.figure(figsize=[12, 4], layout='tight')

axes_1 = figure.add_subplot(1, 2, 1)
axes_2 = figure.add_subplot(1, 2, 2)

# - Precise normal distribution -

axes_1.plot(x, y, label='Analytical PDF')
axes_1.legend()

axes_1.hist(data_precise, bins=100, density=True, histtype='step', label='Empirical PDF')
axes_1.legend()

axes_1.set_title('NormalDistribution<>')

# - Approximate normal distribution -

axes_2.plot(x, y, label='Analytical PDF')
axes_2.legend()

axes_2.hist(data_approx, bins=100, density=True, histtype='step', label='Empirical PDF')
axes_2.legend()

axes_2.set_title('ApproxNormalDistribution<>')

figure.savefig(directory + 'random_approx_distributions.svg')
