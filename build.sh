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
        ./build.sh               : no-args defers to make-debug
        ./build.sh make-debug    : build debug build
        ./build.sh make-release  : build release build
        ./build.sh run           : builds and runs in debug mode. Args after 1st are passed to executable
        ./build.sh r             : make and runs debug without rerunning cmake
        ./build.sh run-release   : builds and runs in release mode.  Args after 1st are passed to executable
        ./build.sh test          : builds and runs tests (in debug mode)
        ./build.sh t             : make and runs debug tests without rerunning cmake
        ./build.sh test-release  : builds and runs tests in release mode.
        ./build.sh install       : Installs the relase version of dynamic_paper
        ./build.sh uninstall     : UnInstalls the relase version of dynamic_paper
        ./build.sh clean         : Delete Releases

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
benchmark_name='dynamic_paper_benchmarking'

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
    target_name="$1"
    debug_or_release="$2"
    if [[ "$#" == 3 ]]; then
        run_cmake="$3"
    else
        run_cmake=""
    fi

    if [[ "$target_name" == "main" ]]; then
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

    case "$target_name" in
        "main")
            make dynamic_paper
            ;;
        "test")
            make dynamic_paper_test
            ;;
        "benchmark")
            make dynamic_paper_benchmarking
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
        "benchmark")
            ./"$benchmark_name"
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

function install {
    echo -e "Installing ${MAGENTA}dynamic_paper${NC}"
    cd "Release"
    echo -e ""
    echo -e "Running ${CYAN}sudo make install${NC}"
    echo -e ""
    sudo make install
    echo -e "${CYAN}Done${NC}"
}

function uninstall {
    echo -e "Uninstalling ${MAGENTA}dynamic_paper${NC}"
    cd "Release"
    echo -e ""
    echo -e "Running ${CYAN}sudo make uninstall${NC}"
    echo -e ""
    sudo make uninstall
    echo -e "${CYAN}Done${NC}"
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
    "benchmark-debug")
        build "benchmark" "debug" 1
        run "benchmark"
        ;;
    "benchmark")
        build "benchmark" "release" 1
        run "benchmark"
        ;;
    "clean")
        if [[ -d Release ]]; then
            rm -rf Release
        else
            echo -e "${RED}nothing to clean O:${NC}"
        fi
        ;;
    "install")
        build "main" "release" 1
        install
        ;;
    "uninstall")
        if ! [[ -d Release/ ]]; then
            build "main" "release" 1
        fi
        uninstall
        ;;
    *)
      echo -e "${RED}Bad arg when parsing cmd line args${NC}"
      exit 1
      ;;
esac
