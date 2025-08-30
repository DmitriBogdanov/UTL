# Reproducing figures from documentation

[<- back to README.md](..)

All plots showcased in this repo are made to be easily reproducible.

To achieve this we use a combination of C++ targets and [Python](https://www.python.org/) scripts residing in the [`auxiliary/`](../auxiliary/) directory.

Auxiliary targets are built together with tests, examples & benchmarks in a similar fashion. These targets can be invoked to generate numeric data, which is then visualized with [matplotlib](https://matplotlib.org/).

## Generating a figure

Build the project according to the [guide](guide_building_project.md).

To generate figures for `<target_name>` run corresponding executable:

```sh
./build/auxiliary/auxiliary-<target_name>
```

and then plot the result:

```sh
python3 auxiliary/<target_name>/plot.py 
```

Generated data & corresponding figures will reside in `temp/<target_name>/`.

> [!Important]
> Last step requires additional dependencies that aren't necessary for the build, see [below](#installing-python-dependencies).

## Installing Python dependencies

`python3` can be downloaded from the [official website](https://www.python.org/downloads/) or using a package manager:

```sh
sudo apt update &&
sudo apt install python3
```

To install necessary dependencies run:

```sh
pip install matplotlib &&
pip install mplcyberpunk
```