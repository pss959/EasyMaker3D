#!/usr/bin/env bash

# Uses createicon.sh to convert MakerVR (Unity) icons to IMakerVR icons.

mvr_dir="/home/pss/git/makervr/MakerVR/Assets/Textures/Resources"
imvr_dir="/home/pss/git/IMakerVR/resources/images"

script_dir="$(dirname ${BASH_SOURCE[0]})"
converter="$script_dir/createicon.sh"

declare -a icons=(
    "Back"
    "Down"
    "EditName"
    "Forward"
    "Help"
    "Home"
    "Settings"
    "Up"
)

for ic in "${icons[@]}"
do
    input="${ic}Icon.png"
    output="${ic}Icon.jpg"
    $converter "$mvr_dir/$input" "$imvr_dir/$output"
    echo "Created $output"
done
