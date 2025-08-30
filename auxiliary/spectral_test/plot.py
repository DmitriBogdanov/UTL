import numpy, matplotlib.pyplot, mplcyberpunk

# --- Import data ---
# -------------------

directory = 'temp/spectral_test/'

data = numpy.genfromtxt(directory + 'randu.csv', delimiter=',')

x = data[:, 0]
y = data[:, 1]
z = data[:, 2]

# --- Plot ---
# ------------

matplotlib.pyplot.style.use('cyberpunk')

figure = matplotlib.pyplot.figure()

axes = figure.add_subplot(projection='3d')

axes.scatter(x, y, z)

# - Angled view (planes visually unnoticeable) -

axes.view_init(elev=30, azim=40, roll=0)
axes.set_box_aspect(aspect=(1, 1, 0.8), zoom=1.1)
axes.set_title('Uniform point distribution from an angle', y=1.10)

figure.savefig(directory + 'random_spectral_test_angle.svg')

# - Top-down view (planes visually noticeable) -

axes.view_init(elev=82, azim=180, roll=0)
axes.set_box_aspect(aspect=(1, 1, 0.8), zoom=1.4)
axes.set_zticks([0.0, 1.0]) # less ticks fit due to the view angle
axes.set_title('Uniform point distribution from above', y=1.15)

figure.savefig(directory + 'random_spectral_test_above.svg')
