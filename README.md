# XenoLib
[![Build Status](https://travis-ci.org/PredatorCZ/XenoLib.svg?branch=master)](https://travis-ci.org/PredatorCZ/XenoLib)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/ca0b675a83e4448cb90ddb858607b9f5)](https://app.codacy.com/app/PredatorCZ/XenoLib?utm_source=github.com&utm_medium=referral&utm_content=PredatorCZ/XenoLib&utm_campaign=Badge_Grade_Dashboard)

XenoLib is independent serialize library for various formats used by Xenoblade Engine.\
Library is compilable under VS 2015, 2017, 2019 and GCC 7, 8.

## Supported formats
* Loading for MXMD (camdo, wimdo) and their stream files.
* DRSM
* MTXT/LBIM conversion into dds/png format
* XBC1 desompressor
* SAR archive
* BC (SKEL, ANIM)

## License
This library is available under GPL v3 license. (See LICENSE.md)

This library uses following libraries:

* libpng, more in libpng/LISENCE
* PreCore, Copyright (c) 2016-2019 Lukas Cone
* zlib, Copyright (C) 1995-2017 Jean-loup Gailly and Mark Adler
