<h1 align="center">
<img src="./images/icon.svg" alt="title">
  <img src="./images/title.svg" alt="title">
</h1>

A dead simple HTTP server written in C

### Table of contents

- [About the Project](#about-the-project)
- [Getting Started](#getting-started)
  - [Build Systems](#build-systems)
  - [Dependencies](#dependencies)
    - [Mandatory](#mandatory)
    - [Optional](#optional)
  - [Building the Project](#building-the-project)
- [Usage](#usage)
- [Running Tests](#running-tests)
- [Supported MIME types](#supported-mime-types)
- [License](#license)

## About the Project

Styx is a lightweight and straightforward HTTP server for Linux written in C, designed to serve static files with minimal configuration. It is designed with local web development in mind, supporting various commonly used [MIME types](https://developer.mozilla.org/en-US/docs/Web/HTTP/MIME_types/Common_types).

## Getting Started

### Build Systems

This is what you will need to build the project:

- [gcc](https://gcc.gnu.org/)
- [CMake](https://cmake.org/) (3.11 or newer)
- [Ninja](https://ninja-build.org/)

### Dependencies

#### Mandatory

- [cJSON](https://github.com/DaveGamble/cJSON) for parsing the server config (automatically installed and linked as a static library via the build script)

#### Optional

- [Criterion](https://github.com/Snaipe/Criterion) to build the test binary
- [gcovr](https://gcovr.com/en/stable/) to generate test coverage

### Building the Project

Assuming you have everything set up, you can simply run

```sh
./build.sh <build-type>
```

with

- --release
- --debug
- --test

being valid build flags. This can generate two different binaries in the build directory, one being the regular program, the other being the tests.

## Usage

> [!IMPORTANT]
> To run the program, you will need two things:
>
> - A JSON config file (a default file comes with the repository)
> - Create a `static` directory, where the servable files reside

Running it without one of these two will result in an error, which gracefully exits the program. However, you do NOT need them to run tests. Config file parsing is very strict, meaning that you can't leave out or add any properties. The default one looks like this:

```json
{
  "port": 8080,
  "ip": "127.0.0.1",
  "recv_header_sz": 8192,
  "recv_body_sz": 2048,
  "resp_header_sz": 16384,
  "resp_body_sz": 4194304,
  "timeout_s": 1,
  "max_clients": 10
}
```

> [!NOTE]
> Every property ending in "sz" is in bytes.

Having set everything up, you can now just run

```sh
./run.sh <optional: config-path>
```

and see for yourself!

## Running Tests

You can run all tests with

```sh
./run_tests.sh <optional: --coverage>
```

> [!NOTE]
> For expected test behavior, ports 8080-8095 need to be free. If this is not possible, you can manually change them inside the test source files at your own risk.
>
> Also, since some tests generate JSON files, there is a clear_jsons.sh script, which you can use, should any of the tests fail to clear them up. It erases any .json files from your current directory, excluding "serverconfig.json".

## Supported MIME types

Here's a list of all MIME types supported by the Styx server:

- html/htm
- css
- js/mjs
- avif
- ico
- jpeg/jpg
- mp3
- mp4
- mpeg
- otf
- ttf
- png
- pdf
- svg
- txt
- wav
- weba
- webm
- webp
- xml
- json/jsonld
- gif

All other file endings are sent under the `application/octet-stream` MIME type.

## License

Distributed under the MIT license. See `LICENSE.txt` for more information.
