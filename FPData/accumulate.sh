#!/bin/bash

function copy()
{
   dir=`pwd`
   mkdir -p ${1}
   cp $dir/Data/* ${1} -R -u
   cp $dir/../FP/Engine/bin-${1}/engine.plugin ${1}/FryingPan/Plugins -u
   cp $dir/../FP/GUI-MUI/bin-${1}/gui.plugin ${1}/FryingPan/Plugins -u
   cp $dir/../FP/bin-${1}/FryingPan ${1}/FryingPan -u
   cp $dir/../Optical/bin-${1}/optical.plugin ${1}/FryingPan/Plugins -u
   cp $dir/../DTLib/bin-${1}/dtlib.plugin ${1}/FryingPan/Plugins -u
   cp $dir/../ISOBuilder/bin-${1}/ISOBuilder.plugin ${1}/FryingPan/Plugins -u
   cp $dir/../Expat/bin-${1}/expat2.library ${1}/FryingPan/Libs -u
   cp $dir/../PlugLib/bin-${1}/plug.library ${1}/FryingPan/Libs -u
  
   find ${1} -type d -name ".svn" -exec rm -rf "{}" \; 2>/dev/null
   cdir=`pwd`
   cd ${1}
   if [ -e FryingPan.lha ]; then
      echo lha uo5 FryingPan.lha FryingPan FryingPan.info Install Install.info
      lha uo5 FryingPan.lha FryingPan FryingPan.info Install Install.info
   else
      echo lha ao5 FryingPan.lha FryingPan FryingPan.info Install Install.info
      lha ao5 FryingPan.lha FryingPan FryingPan.info Install Install.info
   fi
   rm FryingPan.bak
   cd $cdir
}

copy aros/release
copy os3/release
copy mos/release
copy os4/release
copy aros/debug
copy os3/debug
copy mos/debug
copy os4/debug

mkdir -p Arch
cp aros/release/FryingPan.lha Arch/aros_rel.lha
cp mos/release/FryingPan.lha Arch/mos_rel.lha
cp os3/release/FryingPan.lha Arch/os3_rel.lha
cp os4/release/FryingPan.lha Arch/os4_rel.lha
cp aros/debug/FryingPan.lha Arch/aros_dbg.lha
cp mos/debug/FryingPan.lha Arch/mos_dbg.lha
cp os3/debug/FryingPan.lha Arch/os3_dbg.lha
cp os4/debug/FryingPan.lha Arch/os4_dbg.lha
