#!/bin/bash
#I pledge my honor that I have abided by the Stevens Honor System. -Isabella Stone

#checks for the presence of the ~/.junk directory.
mkdir -p ~/.junk 

readonly JUNKDIR=$HOME/.junk

function message {
  cat <<EOF
Usage: $(basename $0) [-hlp] [list of files]
  -h: Display help.
  -l: List junked files.
  -p: Purge all files.
  [list of files] with no other arguments to junk those files.
EOF
}

function purge {
  #echo test
  for file in $JUNKDIR; do
    rm -rf $file
  done
}
  
ARGC=$#
if [[ $ARGC = 0 ]]; then
  message
fi 

COUNT=0
if [[ $ARGC>1 ]]; then
  #if an error needs to be thrown
  #echo test1
  while getopts ":hlp" options; do 
  ((COUNT++))
    case "${options}" in
      h)
        if [[ $ARGC = $COUNT ]] ; then
          #echo argc: $ARGC    count: $COUNT
          echo "Error: Too many options enabled."
          message
          exit 1
        fi
        ;;
      l)
        if [[ $ARGC = $COUNT ]] ; then
          echo "Error: Too many options enabled."
          message
          exit 1
        fi
        ;;
      p)
        if [[ $ARGC = $COUNT ]] ; then
          echo "Error: Too many options enabled."
          message
          exit 1
        fi
        ;;
      *)
      #else invalid option:
        echo "Error: Unknown option '-${OPTARG}'."
        message
        exit 1
    esac
  done
else 
  while getopts ":hlp" options; do 
    case "${options}" in
      h)
        message 
        exit 0
        ;;
      l)
        ls -lAF ~/.junk
        exit 0
        ;;
      p)
        purge
        exit 0
        ;;
      *)
      #else invalid option:
        echo "Error: Unknown option '-${OPTARG}'."
        message
        exit 1
    esac
  done
fi 

#if not given flags, add files/args to folder, if file isn't found warn user, but still move good ones
for arg in $@; do
  if [[ -e "$arg" ]]; then
    mv $arg $JUNKDIR  
  else 
    echo "Warning: '$arg' not found."
 fi
done

