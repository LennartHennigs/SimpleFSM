#!/bin/bash

# SimpleFSM Library Compilation Test Script
# Tests compilation across multiple Arduino platforms and all examples
# Author: Generated for SimpleFSM v2.0
# Date: August 17, 2025

set -e  # Exit on any error

#######################################################################
# CONFIGURATION CONSTANTS
#######################################################################

# Test platforms configuration
# Format: "FQBN|Display Name"
declare -a PLATFORMS=(
    "esp8266:esp8266:d1_mini|Wemos D1 Mini (ESP8266)"
    "esp32:esp32:m5stack_core2|M5Stack Core2 (ESP32)"
    "arduino:avr:nano|Arduino Nano"
)

# Platform-specific example exclusions
# Format: "FQBN:example1,example2,..."
PLATFORM_EXCLUSIONS=(
    "arduino:avr:nano:MixedTransitionsBrowser"  # WiFi/WebServer not available on Nano
)

# Examples directory
EXAMPLES_DIR="examples"

#######################################################################
# DISPLAY CONFIGURATION
#######################################################################

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Get all example directories
EXAMPLES=($(find $EXAMPLES_DIR -name "*.ino" -exec dirname {} \; | sort | uniq))

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    case $status in
        "INFO")
            echo -e "${BLUE}[INFO]${NC} $message"
            ;;
        "SUCCESS")
            echo -e "${GREEN}[PASS]${NC} $message"
            ;;
        "ERROR")
            echo -e "${RED}[FAIL]${NC} $message"
            ;;
        "WARNING")
            echo -e "${YELLOW}[WARN]${NC} $message"
            ;;
    esac
}

# Function to test compilation
test_compilation() {
    local platform_fqbn=$1
    local platform_name=$2
    local example_path=$3
    local example_name=$(basename "$example_path")
    local ino_file="$example_path/$example_name.ino"
    
    # Check for platform-specific exclusions
    for exclusion in "${PLATFORM_EXCLUSIONS[@]}"; do
        local excl_platform=$(echo "$exclusion" | cut -d':' -f1-3)
        local excl_examples=$(echo "$exclusion" | cut -d':' -f4-)
        
        if [[ "$platform_fqbn" == "$excl_platform" ]] && [[ "$excl_examples" == *"$example_name"* ]]; then
            echo -e "  ${YELLOW}[SKIP]${NC} $example_name (platform-specific)"
            return 0
        fi
    done
    
    # Check if .ino file exists
    if [ ! -f "$ino_file" ]; then
        echo -e "  ${YELLOW}[SKIP]${NC} $example_name (file not found)"
        return 0
    fi
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    # Compile with minimal output
    echo -n "  Testing $example_name ... "
    if arduino-cli compile --fqbn "$platform_fqbn" "$ino_file" --output-dir "/tmp/arduino-build-$example_name-$(date +%s)" > /dev/null 2>&1; then
        echo -e "${GREEN}[PASS]${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        echo -e "${RED}[FAIL]${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        
        # Show compilation error for debugging
        echo -e "    ${RED}Error details:${NC}"
        arduino-cli compile --fqbn "$platform_fqbn" "$ino_file" 2>&1 | tail -5 | sed 's/^/     /'
        echo ""
        return 1
    fi
}

# Function to check if arduino-cli is available
check_prerequisites() {
    print_status "INFO" "Checking prerequisites..."
    
    if ! command -v arduino-cli &> /dev/null; then
        print_status "ERROR" "arduino-cli is not installed or not in PATH"
        echo "Please install arduino-cli: https://arduino.github.io/arduino-cli/"
        exit 1
    fi
    
    print_status "SUCCESS" "arduino-cli found: $(arduino-cli version | head -1)"
    echo ""
}

# Function to check installed cores
check_cores() {
    print_status "INFO" "Checking installed Arduino cores..."
    
    local cores_output=$(arduino-cli core list)
    echo "$cores_output"
    echo ""
    
    # Check if required cores are installed
    local missing_cores=()
    
    if ! echo "$cores_output" | grep -q "esp8266:esp8266"; then
        missing_cores+=("esp8266:esp8266")
    fi
    
    if ! echo "$cores_output" | grep -q "esp32:esp32"; then
        missing_cores+=("esp32:esp32")
    fi
    
    if ! echo "$cores_output" | grep -q "arduino:avr"; then
        missing_cores+=("arduino:avr")
    fi
    
    if [ ${#missing_cores[@]} -gt 0 ]; then
        print_status "WARNING" "Missing cores detected: ${missing_cores[*]}"
        print_status "INFO" "Install missing cores with:"
        for core in "${missing_cores[@]}"; do
            echo "  arduino-cli core install $core"
        done
        echo ""
    fi
}

# Function to run all tests
run_all_tests() {
    print_status "INFO" "Starting compilation tests for SimpleFSM library"
    echo "======================================================="
    echo ""
    
    # Test each platform with each example
    for platform_info in "${PLATFORMS[@]}"; do
        IFS='|' read -r platform_fqbn platform_name <<< "$platform_info"
        
        print_status "INFO" "Testing platform: $platform_name"
        echo "----------------------------------------"
        
        for example_path in "${EXAMPLES[@]}"; do
            test_compilation "$platform_fqbn" "$platform_name" "$example_path"
        done
        
        echo ""
    done
}

# Function to display summary
show_summary() {
    echo ""
    echo "======================================================="
    print_status "INFO" "Compilation Test Summary"
    echo "======================================================="
    echo "Total Tests:  $TOTAL_TESTS"
    echo -e "Passed:       ${GREEN}$PASSED_TESTS${NC}"
    echo -e "Failed:       ${RED}$FAILED_TESTS${NC}"
    
    if [ $FAILED_TESTS -eq 0 ]; then
        echo ""
        print_status "SUCCESS" "All compilation tests passed! 🎉"
        echo "The SimpleFSM library is compatible with all tested platforms."
    else
        echo ""
        print_status "ERROR" "Some compilation tests failed!"
        echo "Please review the error messages above and fix any issues."
        exit 1
    fi
}

# Function to display help
show_help() {
    echo "SimpleFSM Library Compilation Test Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  -v, --verbose  Enable verbose output"
    echo "  -q, --quiet    Minimal output (errors only)"
    echo ""
    echo "This script tests compilation of all SimpleFSM examples across:"
    echo "  - Wemos D1 Mini (ESP8266)"
    echo "  - M5Stack Core2 (ESP32)"
    echo "  - Arduino Nano (AVR)"
    echo ""
    echo "Prerequisites:"
    echo "  - arduino-cli must be installed and in PATH"
    echo "  - Required Arduino cores must be installed"
    echo ""
}

# Main execution
main() {
    # Parse command line arguments
    case "${1:-}" in
        -h|--help)
            show_help
            exit 0
            ;;
        -v|--verbose)
            set -x
            ;;
        -q|--quiet)
            exec > /dev/null 2>&1
            ;;
    esac
    
    # Change to script directory
    cd "$(dirname "$0")"
    
    # Run the test suite
    check_prerequisites
    check_cores
    run_all_tests
    show_summary
}

# Execute main function with all arguments
main "$@"
