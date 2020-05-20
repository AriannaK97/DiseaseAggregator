#!/bin/bash
#call with >5 args

generate_random_name(){
  if [ ! -f "$namesFile" ];
  then
    echo "File $namesFile does not exist"
    exit 1;
  elif [ ! -f "$surnamesFile" ];
  then
    echo "File $surnamesFile does not exist"
    exit 1 ;
  else
    FLOOR=1
    NAME_RANGE=$(< "./names" wc -l) #number of lines is the range RANDOM can act
    SURNAME_RANGE=$(< "./surnames" wc -l) #number of lines is the range RANDOM can act

    nameLine=0   #initialize
    surnameLine=0
    while [ "$nameLine" -le $FLOOR ]
    do
      nameLine=$RANDOM
      let "nameLine%=$NAME_RANGE"  # Scales $number down within $DAY_RANGE.
    done

    while [ "$surnameLine" -le $FLOOR ]
    do
      surnameLine=$RANDOM
      let "surnameLine%=$SURNAME_RANGE"  # Scales $number down within $DAY_RANGE.
    done

    name=$(sed "${nameLine}q;d" "$namesFile");
    surname=$(sed "${surnameLine}q;d" "$surnamesFile");

    #code to generate random alphanumerics for names and surnames
    #FLOOR=3
    #let strLength=$RANDOM+$FLOOR;
    #name=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 12 | head -n 1)
    #surname=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 12 | head -n 1)

    echo "$name $surname"
  fi
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

  YEAR_RANGE=2020
  YEAR_FLOOR=1960
  year=0   #initialize
  while [ "$year" -le $YEAR_FLOOR ]
  do
    year=$RANDOM
    let "year %= $YEAR_RANGE"  # Scales $number down within $DAY_RANGE.
  done
  echo "$day-$month-$year"

}

generate_entry_type(){
  BINARY=2
  T=1
  probability=$RANDOM

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
  RANGE=$(< "$diseaseFile" wc -l) #number of lines is the range RANDOM can act
  picker=0
  while [ "$picker" -le $FLOOR ]
  do
    picker=$RANDOM
    let "picker %= $RANGE"  # Scales $number down within $RANGE.
  done
  line=$(sed "${picker}q;d" "$diseaseFile");
  echo "$line"
}

create_new_record(){
  #id=$RANDOM
  id=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 6 | head -n 1)
  entryType=$(generate_entry_type)
  name=$(generate_random_name)
  disease=$(get_random_disease)
  age=$(get_random_age)
  echo "$id" "$entryType" "$name" "$disease" "$age"
#  if [ "$entryType" == "EXIT" ]; then
#    echo "$id" "ENTRY" "$name" "$disease" "$age"
#    echo "$id" "$entryType" "$name" "$disease" "$age"
#  else
#    echo "$id" "$entryType" "$name" "$disease" "$age"
#  fi
}

create_num_files_per_directory(){
  for ((z = 0 ; z < "$numFilesPerDirectory" ; z++));
    do
      create_new_file
  done
}

create_new_file(){
    while true;
    do
      filename=$(generate_random_date)
      if [ ! -f "$filename" ]; then
          break
      fi
    done
    for((j=0; j < "$numRecordsPerFile"; j++)){
      echo -e "$(create_new_record)" >> ./"$inputDir"/"$line/""$filename"
    }
}

echo "All args are = $*";
echo "Number of Parameters = $#"

diseaseFile=$1;
countriesFile=$2;
inputDir=$3;
numFilesPerDirectory=$4;
numRecordsPerFile=$5;
namesFile="./names"
surnamesFile="./surnames"

#checking input numbers
isNum='^[0-9]+$'
if ! [[ $numFilesPerDirectory =~ $isNum ]] || ! [[ $numRecordsPerFile =~ $isNum ]] ;
then
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
echo "$numOfCountries"
for ((i = 1 ; i <= "$numOfCountries" ; i++));
  do
    line=$(sed "${i}q;d" "$countriesFile");
    if [ ! -d "./$inputDir/$line" ]
      then
      echo "Creating new subDirectory in directory $inputDir with name $line"
      mkdir -p "./$inputDir/$line"
      echo "File created"
      create_num_files_per_directory
    else
      echo "File exists"
    fi
done

