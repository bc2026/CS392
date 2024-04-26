#!/bin/bash
# trivia.sh

# Function to display usage using heredoc
usage() {
  cat << EOF
Usage: $0 [-f question_file] [-i IP_address] [-p port_number] [-h]
   -f question_file\tDefault to "question.txt";
   -i IP_address\tDefault to "127.0.0.1";
   -p port_number\tDefault to 25555;
   -h            \tDisplay this help info.
EOF
exit 1
}

f_default='"question.txt"'
i_default='127.0.0.1'
p_default='25555'

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
f_flag=0
i_flag=0
h_flag=0
p_flag=0

# Parse options
while getopts ":fiph" opt; do
  case "${opt}" in
    f)
      ((f_flag++))
      ;;
    i)
      ((i_flag++))
      ;;
    p)
      ((p_flag++))
      ;;
    h) 
        ((h_flag++))
        ;;
    ?)
      error
      ;;
  esac
done

# Prevent multiple flags
if [ $((f_flag + i_flag + p_flag + h_flag)) -gt 3]; then
    
else 




