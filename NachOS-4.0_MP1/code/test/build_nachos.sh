#!/bin/sh

# Default values for flags
QUIET=false
JOBS=1
CLEAN=false

# Show usage/help message
show_help() {
    echo "Usage: $0 [-q] [-j] [-c] [-h|--help]"
    echo
    echo "Options:"
    echo "  -q        Silence output"
    echo "  -j        Run make with multiple jobs (uses all available cores)"
    echo "  -c        Clean before building"
    echo "  -h, --help Display this help message"
    exit 0
}

# Parse flags
while getopts "qjch-:" opt; do
    case $opt in
        q)
            QUIET=true
            ;;
        j)
            JOBS=$(nproc)  # Use all available cores by default
            ;;
        c)
            CLEAN=true
            ;;
        h)
            show_help
            ;;
        -) # Handle long options
            case "${OPTARG}" in
                help)
                    show_help
                    ;;
                *)
                    echo "Invalid option: --${OPTARG}"
                    exit 1
                    ;;
            esac
            ;;
        *)
            echo "Usage: $0 [-q] [-j] [-c] [-h|--help]"
            exit 1
            ;;
    esac
done

# Move to the build directory
cd ../build.linux || exit

# Clean build if -c flag is set
if [ "$CLEAN" = true ]; then
    echo "Cleaning..."
    if [ "$QUIET" = true ]; then
        make clean > /dev/null 2>&1
    else
        make clean
    fi
fi

# Build nachos
echo "Building with $JOBS job(s)..."
if [ "$QUIET" = true ]; then
    make -j"$JOBS" > /dev/null 2>&1
else
    make -j"$JOBS"
fi

# Check build result
if [ $? -eq 0 ]; then
    echo "Build success"
else
    echo "Build failed"
    exit 1
fi
