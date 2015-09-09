opencore-amr-js
===================

AMR-NB Codec in Javascript. Ported [opencore-amr master](http://sourceforge.net/projects/opencore-amr/)
using [emscripten](http://emscripten.org). This project is inspired by [amr.js](https://github.com/jpemartins/amr.js).

Build under Ubuntu
-----

Download and unzip portable emscripten SDK to a directory, say `EMSDK_PORTABLE`:
<http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html>

Install and activate the emscripten SDK under `EMSDK_PORTABLE`:

```
./emsdk update
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
sudo apt-get install nodejs-legacy
```

Install automake and autoconf tools:

`sudo apt-get install libtool automake autoconf`

Run `make` under `js` directory of the project source code tree.

Build under Mac OS X
-----

Install [homebrew](http://brew.sh/).

Download and unzip portable emscripten SDK to a directory, say `EMSDK_PORTABLE`:
<http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html>

Install and activate the emscripten SDK under `EMSDK_PORTABLE`:

```
brew install cmake nodejs python
./emsdk update
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

Install automake and autoconf tools:

`brew install libtool automake autoconf`

Run `make` under `js` directory of the project source code tree.

Demo
-----

The demo file is `js/demo.html` and could also be accessed online from <http://yxl.github.io/opencore-amr-js/>.

Browser Compatibility
-----

Firefox 30+

Chrome 39+

License
-----

Released under the same lincense of Apache License V2.0 as the original project.
