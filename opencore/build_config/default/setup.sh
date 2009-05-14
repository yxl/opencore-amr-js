function echo_stdout()
{
   echo -e "[setup.sh] $@"
}

function print_menu()
{
    echo_stdout
    echo_stdout "Build selection menu... choose from the following:"
    echo_stdout
    echo_stdout "1. Build for host platform"
    echo_stdout "2. Arm device build using OpenCORE (Android) cross-compiler"
    echo_stdout "3. Build using default linux-arm cross-compiler"
    echo_stdout "4. Arm device build using OpenCORE (Android) cross-compiler inside AccuRev workspace"
    echo_stdout

}

function clean_env()
{
  echo_stdout "=================================="
  echo_stdout "Cleaning ARCHITECTURE"
  unset ARCHITECTURE
  echo_stdout "Cleaning ANDROID_BASE"
  unset ANDROID_BASE
  echo_stdout "Setting PATH back to the original"
  export PATH=$BASE_PATH
  echo_stdout "=================================="
}

function menu()
{
    if [ "$1" ] ; then
        CHOICE=$1
    else
        print_menu
        read -p "[setup.sh] Which selection would you like? " CHOICE
    fi

    case $CHOICE in
    1)
        echo_stdout "Choice is to build for the host platform."
        clean_env
        ;;
    2)
        echo_stdout "Choice is to build for target with OpenCORE (Android) cross-compiler"
        ## clean the environment
        clean_env
        ## set path up for linux OpenCore build
        android_gcc_arm_path=/opt/environments/android/toolchain-eabi-4.2.1/bin
        export ARCHITECTURE=android
        echo_stdout "ARCHITECTURE set to ==> $ARCHITECTURE"
        export PATH=$android_gcc_arm_path:$BASE_PATH
        export ANDROID_BASE=/opt/environments/android
        echo_stdout "ANDROID_BASE set to ==> $ANDROID_BASE"
        ;;
    3)
        echo_stdout "Choice is to build for target with the default linux-arm cross-compiler"
        # clean the environment
        clean_env
        # set path up for linux-arm compiler
        linux_arm_path=/opt/environments/linux_arm/data/omapts/linux/arm-tc/gcc-3.4.0-1/bin
        export ARCHITECTURE=linux_arm
        export PATH=$linux_arm_path:$BASE_PATH
        ;;
    4)  
        echo_stdout "Choice is to build for target with workspace's OpenCORE (Android) cross-compiler"
        ## clean the environment
        clean_env
        ## set path up for linux OpenCore build
        android_gcc_arm_path=$BASE_DIR/toolchains/android/toolchain-eabi-4.2.1/bin
        export ARCHITECTURE=android
        echo_stdout "ARCHITECTURE set to ==> $ARCHITECTURE"
        export PATH=$android_gcc_arm_path:$BASE_PATH
        export ANDROID_BASE=$BASE_DIR/toolchains/android
        echo_stdout "ANDROID_BASE set to ==> $ANDROID_BASE"
        ;;
    *)
        echo_stdout "Invalid selection.  Please enter your selection again."
        print_menu
        return
        ;;
    esac
}

function mkcmdcmpl()
{
    printf "\nGetting make cmdline completion values...\n"
    export PV_MAKE_COMPLETION_TARGETS=`make -j completion_targets`
    printf "Done getting make cmdline completion values.\n\n"
}



echo_stdout "started."
echo_stdout "setting up build environment with default configuration"

export PROJECT_DIR=$PWD

if [[ $# -ge 1 ]]; then
  export BASE_DIR=${1%/}
fi

if [[ -z $BASE_DIR ]]; then
  echo_stdout "ERROR: BASE_DIR is not set!"
fi
echo_stdout "BASE_DIR   ==> $BASE_DIR"

export SRC_ROOT=$BASE_DIR
echo_stdout "SRC_ROOT   ==> $SRC_ROOT"

export BUILD_ROOT=$PROJECT_DIR/build
echo_stdout "BUILD_ROOT ==> $BUILD_ROOT"

export CFG_DIR=$PWD
echo_stdout "CFG_DIR    ==> $CFG_DIR"

export MK=$BASE_DIR/tools_v2/build/make
echo_stdout "MK         ==> $MK"

extern_tools_path=$BASE_DIR/extern_tools_v2/bin/linux
export PATH=$extern_tools_path:$PATH
export BASE_PATH=$PATH

_pv_make_completion()
{
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    opts="${PV_MAKE_COMPLETION_TARGETS}"

    case "${prev}" in 
      -f)
        COMPREPLY=( $(compgen -f ${cur}) )
        return 0
        ;;
    *)
        COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
        return 0
        ;;
    esac
}

complete -F _pv_make_completion make
###

echo_stdout
echo_stdout "environment is ready if no errors reported."
echo_stdout "complete."
