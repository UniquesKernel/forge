#!/bin/bash
# Enable recursive globbing
shopt -s globstar

echo "Bootstrapping forge using globstar..."

# g++ will now expand the ** into every .cpp file in the tree
g++ -std=c++20 -O2 -Iinclude bin/forge/src/**/*.cpp -o forge

echo "Done."
