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
$RUN git checkout HEAD

# Merge the master branch to the gh-pages branch.
echo "--- Merging the master branch into the gh-pages branch."
$RUN git merge --no-edit master

# Sync the latest doc into the correct subdirectory.
echo "--- Syncing the HTML doc into the docs/$version subdirectory."
mkdir -p docs
$RUN rsync -vau --delete build/PublicDoc/ "docs/$version/"

# Update the "latest" link.
mkdir -p docs/latest
cat <<EOF > docs/latest/index.html
<!DOCTYPE html>
<html>
  <head>
    <meta http-equiv="Refresh" content="0; url='../$version/index.html'" />
  </head>
  <body>
  </body>
</html>
EOF

# Commit the results.
echo "--- Committing to the gh-pages branch."
$RUN git add "docs/" 
$RUN git commit -m "Updating doc for version $version" -- "docs/" 

# Push to local repo and Github.
echo "--- Pushing to local repo and Github."
$RUN git push origin gh-pages
$RUN git push github gh-pages
