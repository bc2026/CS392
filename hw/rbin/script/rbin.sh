#!/bin/bash

# *******************************************************************************
#  Author  : Bhagawat Chapagain
#  Date    : 02/08/2024
#  Description: CS392 - Homework 1
#  Pledge  : I pledge my honor that I have abided by the Stevens Honor System.
# ******************************************************************************
# Define recycle bin path
readonly recycle_path="$HOME/.recycle"

# Ensure the recycle bin directory exists

mkdir -p "$recycle_path"

# Function to display usage using heredoc
usage() {
  cat << EOF
Usage: rbin.sh [-hlp] [list of files]
   -h: Display this help;
   -l: List files in the recycle bin;
   -p: Empty all files in the recycle bin;
   [list of files] with no other flags,
        these files will be moved to the
        recycle bin.
EOF
  exit 1
}

# Function to handle errors
error() {
  echo "Error: Unknown option '-$OPTARG'." >&2
  usage
}

# Check for no arguments
if [ "$#" -eq 0 ]; then
    usage
fi

# Variables to track flags
h_flag=0
l_flag=0
p_flag=0

# Parse options
while getopts ":hlp" opt; do
  case "${opt}" in
    h)
      ((h_flag++))
      ;;
    l)
      ((l_flag++))
      ;;
    p)
      ((p_flag++))
      ;;
    ?)
      error
      ;;
  esac
done

# Prevent multiple flags
if [ $((h_flag + l_flag + p_flag)) -gt 1 ]; then
  echo "Error: Too many options enabled." >&2
  usage
  exit 1
fi

# Execute based on flags
if [ $h_flag -eq 1 ]; then
  usage
elif [ $l_flag -eq 1 ]; then 
	ls -lAF "$recycle_path/"
	exit 0
elif [ $p_flag -eq 1 ]; then
  rm -rf "$recycle_path"/*
  rm -rf "$recycle_path"/.* 2> /dev/null
  # echo "Recycle bin emptied."
  exit 0
fi

# Move files to the recycle bin
shift $((OPTIND-1))
if [ "$#" -gt 0 ]; then
  for file in "$@"; do
        if [[ -e $file ]]; then
            mv $file $recycle_path/
        else
            echo "Warning: '$file' not found." >&2
        fi
    done
else
  usage
fi
