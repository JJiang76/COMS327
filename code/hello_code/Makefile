# target: list of dependencies
#	rules to build target from dependencies

all: hello times_table

hello: hello.c #hello.h would also be a dependency - delimiter is space
	gcc hello.c -o hello

# hello.class: hello.java - for java file
# javac hello.java
# if dependencies are .o files, all the rule does is linking them together into one binary/executable. - this means switches (eg -Wall, -Werror) dont do anything
times_table: timestable.c times_table.h
	gcc timestable.c -o times_table

# partial compile - does everything except for "linking stage"
# useful for compiling multiple files
# times_table target (above) will then use .o dependencies
# headers wouldnt be included in above
times_table.o: timestable.c times_table.h
	gcc timestable.c -c

clean:
	rm hello times_table

# type 'make' in terminal
# make only builds the default (first) target
# must specify target if building other target
# 'make hello.class'
