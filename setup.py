#!/usr/bin/python
from __future__ import print_function
from setuptools import setup, find_packages

try:
    exec(open("pivotter/version.py").read())
    VERSION = version
except Exception:
    VERSION = ""

NAME = "pivotter"
DESCRIPTION = "Real-time pivot plotter"
AUTHOR = "Søren Bøgeskov Nørgaard"
AUTHOR_EMAIL = "soerenbnoergaard@gmail.com"
URL = "https://github.com/soerenbnoergaard/pivotter"
DOWNLOAD_URL = "https://github.com/soerenbnoergaard/pivotter"
LICENSE = "MIT"

setup(
    name=NAME,
    version=VERSION,
    author=AUTHOR,
    author_email=AUTHOR_EMAIL,
    description=DESCRIPTION,
    url=URL,
    download_url=DOWNLOAD_URL,
    license=LICENSE,

    packages=find_packages(exclude=["docs*", "tests*"]),

    # List of PyPi modules that this package depends on
    install_requires=[
        "numpy",
        "matplotlib",
    ],

    package_data={
    },

    setup_requires=["setuptools_scm"],
    use_scm_version={
        "write_to": "pivotter/version.py",
    },

    extras_require={
    },

    entry_points = {
    },

    zip_safe=True
)
