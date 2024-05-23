# Intel® Video Processing Library (Intel® VPL) Tools

Intel® Video Processing Library (Intel® VPL) tools provide access to hardware
accelerated video decode, encode, and processing capabilities on Intel® GPUs
from the command line.

The tools require the [Intel® VPL base library](https://github.com/intel/libvpl)
and a runtime library installed. Current runtime implementations:


- [Intel® VPL GPU Runtime](https://github.com/intel/vpl-gpu-rt) for use on
  Intel® Iris® Xe graphics and newer
- [Intel® Media SDK](https://github.com/Intel-Media-SDK/MediaSDK) for use on legacy
  Intel graphics

Follow the instructions on the respective repos to install the desired
implementation.

## Build from Source

Building the tools requires an installation of Intel® VPL development package.

### Build and install the Intel® VPL development package

Linux:
```
git clone https://github.com/intel/libvpl
pushd libvpl
export VPL_INSTALL_DIR=`pwd`/../_vplinstall
sudo script/bootstrap
cmake -B _build -DCMAKE_INSTALL_PREFIX=$VPL_INSTALL_DIR
cmake --build _build
cmake --install _build
popd
```

Windows cmd prompt:
```
git clone https://github.com/intel/libvpl
pushd libvpl
set VPL_INSTALL_DIR=%cd%\..\_vplinstall
script\bootstrap.bat
cmake -B _build -DCMAKE_INSTALL_PREFIX=%VPL_INSTALL_DIR%
cmake --build _build --config Release
cmake --install _build --config Release
popd
```
> **Note:** bootstrap.bat requires [WinGet](https://github.com/microsoft/winget-cli)

### Build and install the Intel® VPL tools

Linux:
```
git clone https://github.com/intel/libvpl-tools
pushd libvpl-tools
export VPL_INSTALL_DIR=`pwd`/../_vplinstall
sudo script/bootstrap
cmake -B _build -DCMAKE_PREFIX_PATH=$VPL_INSTALL_DIR
cmake --build _build
cmake --install _build --prefix $VPL_INSTALL_DIR
```

Windows cmd prompt:
```
git clone https://github.com/intel/libvpl-tools
pushd libvpl-tools
set VPL_INSTALL_DIR=%cd%\..\_vplinstall
script\bootstrap.bat
cmake -B _build -DCMAKE_PREFIX_PATH=%VPL_INSTALL_DIR%
cmake --build _build --config Release
cmake --install _build --config Release --prefix %VPL_INSTALL_DIR%
```
> **Note:** bootstrap.bat requires [WinGet](https://github.com/microsoft/winget-cli)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file
for details.

## Security

See the [Intel® Security
Center](https://www.intel.com/content/www/us/en/security-center/default.html)
for information on how to report a potential security issue or vulnerability.

## How to Contribute

See [CONTRIBUTING.md](CONTRIBUTING.md) for more information.
