#!/bin/bash
# rbin.sh
# Recycle Bin Homework 1
# Bhagawat Chapagain
# I pledge my honor that I had abided by the Stevens Honor System.
#!/bin/bash

usage_msg='Usage: rbin.sh [-hlp] [list of files]
  -h: Display this help;
  -l: List files in the recycle bin;
  -p: Empty all files in the recycle bin;
  [list of files] with no other flags,
  these files will be moved to the recycle bin.'

usage() {
  echo "$usage_msg" 1>&2
  exit 1
}

error() { 

  msg="Error: "
  if [ "$#" -gt 1 ]; then
    msg+="Too many options enabled"
  else
    msg+="Unknown option '$1'."
  fi

  echo "$msg" 1>&2

  echo $usage_msg
  exit 1
}

while getopts ":hlp" o; do
  case "${o}" in
    h)
      usage
      ;;
    l)
      if [ $OPTIND -gt 1 ]; then 
    	error -l -x
    	exit 1
      fi

  echo "Listing files in the recycle bin..."
       ;;
    p)
      echo "Emptying the recycle bin..."
      # Add your code to empty the recycle bin here
      ;;
    *)
      error $*
      	    ;;
  esac
done

# After processing options, you can handle the remaining arguments (files) here
shift $((OPTIND-1))


