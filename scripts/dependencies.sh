#!/bin/bash

# This is the maximum depth to which dependencies are resolved
MAXDEPTH=24

# Get the filename witout extension from a path
# Usage: createname [PATH]
function createname
{
    local STR=$(basename $1 | awk -F "." '{ print $1 }' | sed 's/-//g')
    echo $STR
}

# analyze a given file on its
# dependecies using ldd and write
# the results to a given temporary file
#
# Usage: analyze [OUTPUTFILE] [INPUTFILE] 
function analyze
{
    local OUT=$1
    local IN=$2
    local NAME=$(createname $IN)

    for i in $LIST
    do
	if [ "$i" == "$NAME" ];
	then
   	    # This file was already parsed
	    return
	fi
    done
    # Put the file in the list of all files
    LIST=$(echo -e "$LIST\n$NAME")
	
    DEPTH=$[$DEPTH + 1]
    if [ $DEPTH -ge $MAXDEPTH ];
	then
	echo "MAXDEPTH of $MAXDEPTH reached at file $IN."
	echo "Continuing with next file..."
	# Fix by Marco Nellisen for the case that MAXDEPTH was reached
	DEPTH=$[$DEPTH - 1]
	return
    fi

    echo "Parsing file:              $IN"

    ldd $IN &> $LDDTMPFILE
    RET=$?
    if [ $RET != 0 ];
	then
	echo "ERROR: ldd returned error code $RET"
	echo "ERROR:"
	cat $TMPFILE
	echo "Aborting..."
	rm $TMPFILE
	exit 1
    fi

    # DEPENDENCIES=$(cat $TMPFILE | awk -F " " '{ if( $3 == "" ) print $1; else print $3; }')
    # DEPENDENCIES=$(cat $LDDTMPFILE | awk -F " " '{ print $3; }')
    DEPENDENCIES=$(cat $LDDTMPFILE | awk -F " " '{ if (!match($3, /\(.*\)/) && substr( $0, 1, 13 ) != "ldd: warning:") print $3; }')

    for DEP in $DEPENDENCIES;
    do
	if [ -n "$DEP" ];
	then
	    echo -e "  \"$NAME\" -> \"$(createname $DEP)\";" >> $OUT
	    analyze $OUT "$DEP"
	fi
    done

    DEPTH=$[$DEPTH - 1]
}

########################################
# main                                 #
########################################

if [ $# != 2 ];
    then
    echo "Usage:"
    echo "  $0 [filename] [outputimage]"
    echo ""
    echo "This tools analyses a shared library or an executable"
    echo "and generates a dependency graph as an image."
    echo ""
    echo "GraphViz must be installed for this tool to work."
    echo ""
    exit 1
fi

DEPTH=0
INPUT=$1
OUTPUT=$2
TMPFILE=$(mktemp -t)
LDDTMPFILE=$(mktemp -t)
LIST=""

if [ ! -e $INPUT ];
    then
    echo "ERROR: File not found: $INPUT"
    echo "Aborting..."
    exit 2
fi

echo "Analyzing dependencies of: $INPUT"
echo "Creating tmp file as:      $TMPFILE"
echo "Creating output as:        $OUTPUT"
echo ""

echo "digraph DependecyTree {" > $TMPFILE
echo "  $(createname $INPUT) [shape=box];" >> $TMPFILE
analyze $TMPFILE "$INPUT"
echo "}" >> $TMPFILE

#cat $TMPFILE # output generated dotfile for debugging purposses
dot -Tpng $TMPFILE -o$OUTPUT

rm $LDDTMPFILE
rm $TMPFILE

exit 0

