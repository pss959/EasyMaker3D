# Installs Ion headers and libraries into a target directory.

declare mode="dbg"  # Default is debug mode
if [[ $# == 1 ]]; then
   mode=$1
fi

if [[ "$OSNAME" == "linux" ]] ; then
    declare -r source_root=/local/github/ion
    declare -r target_root=/local/inst/ion
    declare -r os_dir="linux"
    declare -r lib_suffix="*.a"
else
    declare -r source_root=/d/bitbucket/ion
    declare -r target_root=/c/local/ion
    declare -r os_dir="win-ninja"
    declare -r lib_suffix="*.lib"
fi

cd ${source_root}/ion

declare -r mode_target=${target_root}/${mode}
declare -r ext_hdr_source=../
declare -r ion_hdr_source=.
if [[ "$OSNAME" == "linux" ]] ; then
    declare -r lib_source=../gyp-out/${os_dir}/${mode}
else
    declare -r lib_source=../gyp-out/${os_dir}/${mode}_x64
fi
declare -r demo_source=${lib_source}/demos
declare -r doc_source=./doxygen/html

declare -r ext_hdr_dirs="base util"
declare -r ion_hdr_dirs="analytics base base/stlalloc external/gtest gfx
                         gfxprofile gfxutils image math port portgfx profile
                         remote text"

# Install header files.
mkdir -p ${mode_target}/include/ion
for dir in ${ext_hdr_dirs} ; do
    rsync -au --info=name --include="*.h" --exclude="*" \
          ${ext_hdr_source}/${dir}/ ${mode_target}/include/${dir}
done
for dir in ${ion_hdr_dirs} ; do
    td=${mode_target}/include/ion/${dir}
    mkdir -p ${td}
    rsync -au --info=name --include="*.h" --include="*.inc" --exclude="*" \
          ${ion_hdr_source}/${dir}/ ${td}
done

# Special-case ION override headers.
mkdir -p  ${mode_target}/include/ion/port/override
rsync -au --info=name \
      ${ion_hdr_source}/port/override/ ${mode_target}/include
rsync -au --info=name \
      ${ion_hdr_source}/port/override/ ${mode_target}/include/ion/port/override

# Special-case third-party headers. ABSL has extra weirdness.
declare -r tps=${ext_hdr_source}/third_party
declare -r tpd1=${mode_target}/include
declare -r tpd2=${tpd1}/third_party
mkdir -p  ${tpd2}/GL/gl
mkdir -p  ${tpd2}/absl
mkdir -p  ${tpd1}/absl/base
rsync -au --info=name ${tps}/GL/gl/ ${tpd2}/GL/gl
rsync -au --info=name ${tps}/absl/  ${tpd2}/absl
rsync -au --info=name ${tps}/absl/absl/base/macros.h ${tpd1}/absl/base
rsync -au --info=name ${tps}/google/absl/            ${tpd1}/absl

# Install libraries.
mkdir -p ${mode_target}/lib
rsync -au --delete --info=name --include="${lib_suffix}" --exclude="*" \
      ${lib_source}/ ${mode_target}/lib

# Install demos.
mkdir -p ${mode_target}/bin
rsync -au --delete --info=name ${demo_source}/ ${mode_target}/bin

# Install documentation if available.
if [ -d $doc_source ] ; then
   mkdir -p ${target_root}/doc
   rsync -au --delete --info=name ${doc_source}/ ${target_root}/doc
fi
