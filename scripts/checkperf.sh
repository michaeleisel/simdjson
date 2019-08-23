#!/bin/bash

set -e
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

# Arguments: perfdiff.sh <branch> <test json files>
if [ -z "$1" ]; then reference_branch="master"; else reference_branch=$1; shift; fi
if [ -z "$*" ]; then perftests="jsonexamples/twitter.json"; else perftests=$*; fi

# Clone and build the reference branch's parse
echo "Cloning and build the reference branch ($reference_branch) ..."
current=$SCRIPTPATH/..
reference=$current/benchbranch/$reference_branch
rm -rf $reference
mkdir -p $reference
git clone --depth 1 -b $reference_branch https://github.com/lemire/simdjson $reference
cd $reference
make parse

# Build the current branch's parse
echo "Building the current branch ..."
cd $current
make clean
make parse

# Run them and diff performance
make perfdiff

echo "Running perfdiff:"
echo ./perfdiff \"$current/parse -t $perftests\" \"$reference/parse -t $perftests\"
./perfdiff "$current/parse -t $perftests" "$reference/parse -t $perftests"
