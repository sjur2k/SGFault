# SGFault compiler project #

In this project I am exploring how compiled languages turn from written text to executable binaries. \
The target is to develop a minimal programming language which can be compiled  like `./sgfault myfile.sg` \
and then ran like `./myfile` similar to the gcc workflow.\
\
Since this is only a side project this will only target x86-64 architectures and it will make use of\
NASM and the GNU linker ld to get libraries working. The compiler will be written in C.\
\
The motivation for doing this, is to get a deeper understanding of low level programming and to become\
more fluent in C. There is no end-goal for this project, but it is not intended to revolutionize anything.\
It is merely a practice project done for fun.

## Installation for local user ##
`git clone https://github.com/sjur2k/SGFault.git`\
`cd sgfault`\
`(sudo) make install`
