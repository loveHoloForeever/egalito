#!/bin/bash
program=${1:-../binary/build/hello}
name=`basename $program`

rm -f tmp/$name-relocs-unsorted.txt \
  tmp/$name-relocs.txt \
  tmp/$name-q-relocs.txt
mkdir -p tmp
EGALITO_DEBUG=/dev/null EGALITO_CHECK=1 ../../app/etshell \
	2>tmp/$name-relocs-unsorted.txt \
  >tmp/$name.out << EOF
parse $program
dumplinks module-(executable)
EOF

cat tmp/$name-relocs-unsorted.txt | awk '{ print $1, $2 }' | \
  sort -n | uniq > tmp/$name-relocs.txt

./relocs.pl $program-q | awk '{ print $1, $2 }' | sort -n | uniq > tmp/$name-q-relocs.txt

diff -y tmp/$name-q-relocs.txt tmp/$name-relocs.txt
#diff tmp/$name-q-relocs.txt tmp/$name-relocs.txt