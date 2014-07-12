#!/bin/sh
if [ -w 'src/gitrev.h' ]; then
	rm 'src/gitrev.h'
fi

echo \
"#ifndef _GITREV_H\
#define _GITREV_H\
const char gitrev[] = \"`git rev-parse HEAD`\";\
#endif // _GITREV_H\
" > ../gitrev.h
