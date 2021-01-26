typeset -i curr_ver=$(cat version)
((curr_ver=curr_ver+1))
echo $curr_ver > version 
