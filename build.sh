#!/usr/bin/env bash
set -euo pipefail

#
# ===== Help Function ===============
#

RED='\033[0;31m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
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
    if [[ -d "$1" ]]; then
        rm -r "$1"
    fi
    cp -r files "$1"
}

# update `compile_commands.json` in root with the one in release
function bump_compile_commands_json {
    if [[ -f "compile_commands.json" ]]; then
        rm compile_commands.json
    fi
    cp "Release/compile_commands.json" "./"
}

# $1: should be `main` to run main executable `test` to run tests. Can be `all` to build both and
# run tests
# $2: should be `debug` or `release` which determines which type of build to perform
# $3: (optional) `1` if should run cmake
function build {
    main_or_test="$1"
    debug_or_release="$2"
    if [[ "$#" == 3 ]]; then
        run_cmake="$3"
    else
        run_cmake=""
    fi

    if [[ "$main_or_test" == "main" ]]; then
        echo -e "${CYAN}Building main project${NC}"
    else
        echo -e "${MAGENTA}Building project tests${NC}"
    fi

    # Build targets into release
    mkdir -p Release
    cd Release

    if [[ (( "$#" < 4 )) ]]; then
        if [[ "$run_cmake" == "1" ]]; then
            case "$debug_or_release" in
                "debug")
                    cmake -DCMAKE_BUILD_TYPE=Debug ..
                    ;;
                "release")
                    cmake -DCMAKE_BUILD_TYPE=Release ..
                    ;;
                *)
                    echo -e "${RED}Error in build; bad arg for debug/release build${NC}"
                    exit 1
                    ;;
            esac
        fi
    else
        echo -e "${RED}Error in build; bad arg${NC}"
        exit 1
    fi

    case "$main_or_test" in
        "main")
            make dynamic_paper
            ;;
        "test")
            make dynamic_paper_test
            ;;
        "all")
            make all
            ;;
        *)
            echo -e "${RED}error in build; bad arg for whether to build main or test target"
            ;;
    esac

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
        build "all" "debug" 1
        ;;
    "make-release")
        build "all" "release" 1
        ;;
    "run")
        build "main" "debug" 1
        shift
        run "run" "$@"
        ;;
    "r")
        build "main" "debug"
        shift
        run "run" "$@"
        ;;
    "run-release")
        build "main" "release" 1
        shift
        run "run" "$@"
        ;;
    "test")
        build "test" "debug" 1
        run "test"
        ;;
    "t")
        build "test" "debug"
        shift
        run "test" "$@"
        ;;
    "test-release")
        build "test" "release" 1
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
