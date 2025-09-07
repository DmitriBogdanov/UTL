# Building documentation

[<- back to README.md](..)

This repo uses [MkDocs](https://www.mkdocs.org/) with [Material](https://squidfunk.github.io/mkdocs-material/) theme to build the website version of documentation.

Documentation can be build locally through a rather simple process described below.

> [!Important]
> Building documentation website is entirely optional, all docs are made to work locally as regular markdown files connected through relative links. Building it as a website does, however, provide a nicer experience.

## Installing dependencies

`python3` can be downloaded from the [official website](https://www.python.org/downloads/) or using a package manager:

```sh
sudo apt update &&
sudo apt install python3
```

To install necessary dependencies run:

```sh
pip install mkdocs-material &&
pip install mkdocs-material[imaging] &&
pip install markdown-callouts &&
pip install mkdocs-awesome-nav
```

> [!Note]
> When building for public deployment, [social preview plugin](https://squidfunk.github.io/mkdocs-material/plugins/social/) also requires [Cairo Graphics](https://squidfunk.github.io/mkdocs-material/plugins/requirements/image-processing/) libraries. These libraries come preinstalled on [Github Actions (Ubuntu)](https://github.com/actions/runner-images), otherwise they can be manually downloaded with `apt`.

## Building the docs

To build documentation locally navigate to the project directory and run:

```sh
mkdocs serve
```

The link to open local documentation in the browser will appear in the terminal after the build.