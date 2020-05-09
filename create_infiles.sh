#!/bin/bash
#call with >5 args

generate_random_name(){
  FLOOR=3
  let strLength=$RANDOM+$FLOOR;
  name=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 12 | head -n 1)
  surname=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 12 | head -n 1)
  echo "$name $surname"
}

generate_random_date(){
  DAY_RANGE=30
  FLOOR=1
  day=0   #initialize
  while [ "$day" -le $FLOOR ]
  do
    day=$RANDOM
    let "day %= $DAY_RANGE"  # Scales $number down within $DAY_RANGE.
  done

  MONTH_RANGE=12
  FLOOR=1
  month=0   #initialize
  while [ "$month" -le $FLOOR ]
  do
    month=$RANDOM
    let "month %= $MONTH_RANGE"  # Scales $number down within $DAY_RANGE.
  done

  #echo "Random number between $FLOOR and $MONTH_RANGE ---  $month"

  YEAR_RANGE=2020
  YEAR_FLOOR=1960
  year=0   #initialize
  while [ "$year" -le $YEAR_FLOOR ]
  do
    year=$RANDOM
    let "year %= $YEAR_RANGE"  # Scales $number down within $DAY_RANGE.
  done

  #echo "Random number between $YEAR_FLOOR and $YEAR_RANGE ---  $year"

  echo "$day-$month-$year"

}

generate_entry_type(){
  BINARY=2
  T=1
  number=$RANDOM

  let "probability %= $BINARY"

  if [ "$probability" -eq $T ]
  then
    echo "EXIT"
  else
    echo "ENTRY"
  fi
}

get_random_age(){
  RANGE=120
  FLOOR=0
  age=0

  while [ "$age" -le $FLOOR ]
  do
    age=$RANDOM
    let "age %= $RANGE"  # Scales $number down within $RANGE.
  done
  echo "$age"
}

get_random_disease(){
  FLOOR=1;
  numOfLines=$(< "$diseaseFile" wc -l)
  echo "the file has $numOfLines"
  picker=$((RANDOM%=$numOfLines));
  picker=$(picker+=$FLOOR);
  line=$(sed "${picker}q;d" "$diseaseFile");
  echo "$line"
}

create_new_record(){
  id=$RANDOM
  echo $id generate_entry_type generate_random_name get_random_disease get_random_age
}

create_new_file(){
    while true; do
      filename=$(generate_random_date)
      if [ ! -f "$filename" ]; then
          break
      fi
    done
    for((i=0; i < $5; i++)){
      echo create_new_record > ./$3/$line
    }
}

echo "All args are = $*";
echo "Number of Parameters = $#"

diseaseFile=$1;
countriesFile=$2;
inputDir=$3;
numFilesPerDirectory=$4;
numRecordsPerFile=$5;

#checking input numbers
isNum='^[0-9]+$'
if ! [[ $numFilesPerDirectory =~ $isNum ]] || ! [[ $numRecordsPerFile =~ $isNum ]] ; then
    >&2 echo "Error! Wrong Parameters, beeter luck next time"
    exit 1
fi

#make inputDir
if [ ! -d "$inputDir" ]
then
    echo "Creating new directory with name $inputDir"
    mkdir -p "$inputDir"
    echo "File created"
else
    echo "File exists"
fi

#create subdirectories from countries
numOfCountries=$(< "$countriesFile" wc -l)
echo $numOfCountries
for ((i = 0 ; i < $numOfCountries ; i++));
  do
    line=$(sed "${i}q;d" "$countriesFile");
    if [ ! -d "./$inputDir/$line" ]
      then
      echo "Creating new subDirectory in directory $inputDir with name $line"
      mkdir -p "./$inputDir/$line"
      echo "File created"
    else
      echo "File exists"
    fi
done

generate_random_date
generate_entry_type
generate_random_name
get_random_disease
get_random_age

#for str
#  do
#    echo "The value of the iterator is: ${str}"
#    var=$1
#    shift
#    echo "var = $var and args = $#"
#    done
