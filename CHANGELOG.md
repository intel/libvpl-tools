# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Intel® Video Processing Library (Intel® VPL) tools provide access to hardware
accelerated video decode, encode, and frame processing capabilities on Intel®
GPUs from the command line.

## [Unreleased]

## [1.4.0] - 2024-04-11

### Added
- Intel® Media Transcode Accelerator as a supported device
- New strings in vpl-inspect tool to improve readability of output
- Option `-props` to vpl-inspect tool to enable property-based capabilities
  query

### Changed
- Default Ubuntu build to 24.04

## [1.3.0] - 2024-12-13

### Added
- Screen content coding tools for AV1 to `sample_encode`
- GTK renderer option to `sample_decode` and `sample_multi_transcode`
- `-fullscreen` option to `sample_decode` and `sample_multi_transcode` when
  using GTK.  Use <kbd>Ctrl</kbd>+<kbd>f</kbd> to enter fullscreen and
  <kbd>Esc</kbd> to exit fullscreen

- Improved support for Python 3.12 development environments.

### Fixed
- Bootstrap to support Debian distributions that do not define `ID_LIKE`

## [1.2.0] - 2024-08-30

### Added
- VVC decode support to `sample_decode`
- Embedded version information to all shared libraries

### Changed
- Metrics monitor library to now build statically by default

## [1.1.0] - 2024-06-28

### Added
- `MFX_SURFACE_TYPE_VULKAN_IMG2D` to vpl-inspect
- YUV400 JPEG Enc for Linux VAAPI

### Fixed
- va-attrib for vaapiallocator
- D3D11 texture not being released in `val-surface-sharing` test tool

## [1.0.0] - 2024-04-26

### Added
- Intel® VPL API 2.11 support
- Command line tools. They have been moved from the libvpl repository
  (https://github.com/intel/libvpl)


[Unreleased]: https://github.com/intel/libvpl/compare/v1.4.0...HEAD
[1.4.0]: https://github.com/intel/libvpl/compare/v1.3.0...v1.4.0
[1.3.0]: https://github.com/intel/libvpl/compare/v1.2.0...v1.3.0
[1.2.0]: https://github.com/intel/libvpl/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/intel/libvpl/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/intel/libvpl/releases/tag/v1.0.0
