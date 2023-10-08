#!/usr/bin/env bash
set -euo pipefail

#
# ===== Help Function ===============
#

RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

function help_message {
    usage="Usage: $(basename "$0") [command]

    Build script to build this project

    command:
        ./build               : no-args defers to make-debug
        ./build make-debug    : build debug build
        ./build make-release  : build release build
        ./build run           : builds and runs in debug mode. Args after 1st are passed to executable
        ./build r             : make and runs debug without rerunning cmake
        ./build run-release   : builds and runs in release mode.  Args after 1st are passed to executable
        ./build test          : builds and runs tests (in debug mode)
        ./build t             : make and runs debug tests without rerunning cmake
        ./build test-release  : builds and runs tests in release mode.
        ./build clean         : Delete Releases

    flags:
       -h (--help)          Help prompt
"
    echo "$usage"
}

#
# ===== Variables ===============
#

executable_name='dynamic_paper'
test_name='dynamic_paper_test'

#
# ===== Functions ===============
#


# $1: destinaition to copy to
function copy_assets {
    if ! [[ -f "$1" ]]; then
        cp -r files "$1"
    fi
}

# update `compile_commands.json` in root with the one in release
function bump_compile_commands_json {
    if [[ -f "compile_commands.json" ]]; then
        rm compile_commands.json
    fi
    cp "Release/compile_commands.json" "./"
}

# $1: should be `debug` or `release` which determines which type of build to perform
# $2: (optional) `1` if should not run cmake
function build {
    echo -e "${CYAN}Building project${NC}"
    # Build targets into release
    mkdir -p Release
    cd Release
    if [[ (( "$#" == 1 )) || ( (( "$#" -gt 1 )) && "$2" != "1" ) ]]; then
        case "$1" in
            "debug")
                cmake -DCMAKE_BUILD_TYPE=Debug ..
                ;;
            "release")
                cmake -DCMAKE_BUILD_TYPE=Release ..
                ;;
            *)
                echo -e "${RED}Error in build; bad arg${NC}"
                exit 1
                ;;
        esac
    fi
    make

    cd ..
    copy_assets "Release/bin/files"
    bump_compile_commands_json
}

# $1: should be `test` or `run` to determine which to run
# $2 and on: args to pass to process. Is IGNORED if its test
function run {
    echo -e "${CYAN}Running code${NC}"
    cd "Release/bin"
    case "$1" in
        "test")
            ./"$test_name"
            ;;
        "run")
            if (( "$#" > 1 )); then
                shift
                ./"$executable_name" "$@"
            else
                ./"$executable_name"
            fi
            ;;
    esac
    cd ../../
}

#
# ===== Main Script ===============
#


if (( "$#" == 0 )); then
    arg=""
else
    arg="$1"
fi


case "$arg" in
    "-h" | "--help" | "help")
        help_message
        exit 0
        ;;
    "" | "make-debug")
        build "debug"
        ;;
    "make-release")
        build "release"
        ;;
    "run")
        build "debug"
        shift
        run "run" "$@"
        ;;
    "r")
        build "debug" 1
        shift
        run "run" "$@"
        ;;
    "run-release")
        build "release"
        shift
        run "run" "$@"
        ;;
    "test")
        build "debug"
        run "test"
        ;;
    "t")
        build "debug" 1
        shift
        run "test" "$@"
        ;;
    "test-release")
        build "release"
        run "test"
        ;;
    "clean")
        if [[ -d Release ]]; then
            rm -r Release
        else
            echo -e "${RED}nothing to clean O:${NC}"
        fi
        ;;
    *)
      echo -e "${RED}Bad arg when parsing cmd line args${NC}"
      exit 1
      ;;
esac
