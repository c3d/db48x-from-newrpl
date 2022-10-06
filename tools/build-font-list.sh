#!/bin/bash
echo "#ifndef FONTLIST_H"
echo "#define FONTLIST_H"
cat $1
shift
for F in $@
do
    echo "extern const UNIFONT FONT_$F;"
    echo "#define Font_$F (&FONT_$F)"
done
echo "#endif // FONTLIST_H"
