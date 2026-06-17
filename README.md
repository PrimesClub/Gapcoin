# Gapcoin Core

![Gapcoin Logo](https://gapcoin.club/Logos/Gapcoin128.png)

This repository hosts the Gapcoin Core source code, directly based on [Riecoin Core](https://riecoin.xyz/Core/). Gapcoin is a Bitcoin like peer-to-peer currency but with useful Proof of Work, using Prime Gaps instead of hashes for its PoW.

Visit [Gapcoin.club](https://gapcoin.club/) to learn more.

## Build Gapcoin Core

### Recent Debian/Ubuntu

Get the dependencies

```bash
apt install build-essential cmake pkg-config bsdmainutils python3
apt install libevent-dev libboost-system-dev libboost-filesystem-dev libboost-test-dev libboost-thread-dev qt6-base-dev qt6-tools-dev qt6-l10n-tools qt6-wayland libgmp-dev libsqlite3-dev libqrencode-dev libcapnp-dev capnproto
```

Get the source code.

```bash
git clone https://github.com/PrimesClub/Gapcoin.git
```

Build with Qt Gui,

```bash
cd Gapcoin
cmake -B build -DBUILD_GUI=ON
cmake --build build
```

The Gapcoin-Qt binary is located in `build/bin`. You can run `strip gapcoin-qt` to reduce its size a lot, or build without the Qt Gui with

```bash
cd Gapcoin
cmake -B build
cmake --build build
```

The build can be speed up by appending `-j N` to the last command, which runs N parallel jobs.

#### Guix Build

Gapcoin can be built using Guix. The process is longer, but also deterministic: everyone building this way should obtain the exact same binaries. Distributed binaries are produced this way, so anyone can ensure that they were not created with an altered source code by building themselves using Guix.

You should have a lot of free disk space (at least 40 GB), and 16 GB of RAM or more is recommended. Read the [Guix Guide](contrib/guix/README.md) for installation instructions and more.

Now, get the Gapcoin Core source code.

```bash
git clone https://github.com/PrimesClub/Gapcoin.git
```

Start the Guix build. The environment variable will set which binaries to build (here, Linux x64, Linux Arm64, and Windows x64, but it is possible to add other architectures or Mac with an SDK).

```bash
export HOSTS="x86_64-linux-gnu aarch64-linux-gnu x86_64-w64-mingw32"
cd Gapcoin
./contrib/guix/guix-build
```

It will be very long, do not be surprised if it takes an hour or more, even with a powerful machine, though subsequent builds will be faster as some steps are cached. The binaries will be generated in a `guix-build-.../output` folder.

### Other OSes

Either build using Guix as explained above in a spare physical or virtual machine, or refer to the [Bitcoin's Documentation (build-... files)](https://github.com/bitcoin/bitcoin/tree/master/doc) and adapt the instructions for Gapcoin if needed.

## Testing

Tests were not yet adjusted for Gapcoin.

## License

The Gaocoin Core code is published under the terms of the MIT license. See [COPYING](COPYING) for more information or see https://opensource.org/licenses/MIT.

However, releases are under the terms of the Gnu General Public License Version 3 (GPLv3) since Gapcoin Core uses some GPL licensed software.

Please do not profit off the permissive licenses to create yet more cryptos, there are already way too many of these out there. If you like Bitcoin, Riecoin or Gapcoin, then support these Projects directly, contribute Code, mine and hold these coins. Otherwise, surely there is already one out there suiting your purposes, or one close enough with maintainers happy to work out with you. Every new coin naturally becomes yet another rival to existing ones that divides further resources by needing separate maintenance and upgrades, sites, explorers, listing payments, dilutes the value of existing holdings and the mining power, etc. when people could instead join forces. For these reasons, we adopt a Maximalist philosophy.
