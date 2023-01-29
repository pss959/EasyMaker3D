#!/usr/bin/env bash

# -----------------------------------------------------------------------------
# Publishes the current public documentation to Github Pages in the gh-pages
# branch under the "docs/<version-string>/" subdirectory.
# -----------------------------------------------------------------------------

# Set to "echo" for testing.
RUN=

# Get the root directory as an absolute path.
script_dir=$(dirname -- "${BASH_SOURCE[0]}")
root_dir=$(realpath "$script_dir/..")
cd $root_dir

# Get the current version string.
version=$(grep "version_string =" "SConstruct" | \
              sed -e "s/.*= //" | sed -e "s/'//g")

# Switch to the master branch.
echo "--- Switching to the master branch."
$RUN git checkout master

# Make sure the doc is up to date.
echo "--- Building the doc for version $version."
$RUN scons PublicDoc

# Switch to the gh-pages branch.
echo "--- Switching to the gh-pages branch."
$RUN git checkout gh-pages

# Sync the latest doc into the correct subdirectory.
echo "--- Syncing the HTML doc into the docs/$version subdirectory."
$RUN rsync -au --delete build/PublicDoc/ "docs/$version/"

# Commit the results.
echo "--- Committing to the gh-pages branch."
$RUN git commit -m "Updating doc for version $version" -- "docs/$version/" 
