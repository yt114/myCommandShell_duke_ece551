CFLAGS=-Wall -Werror -std=gnu++98 -pedantic -ggdb3
myShellSF=my_shell_class.cpp my_shell_class.h myShell.cpp  
myechoSF=myecho.cpp
LargeData=large_data.cpp
sortLines=sortLines.c
PROGRAM=myecho myShell LargeData sortLines
all:$(PROGRAM)
LargeData: $(LargeData)
	g++ $(CFLAGS) -o $@ $(LargeData)
myecho: $(myechoSF)
	g++ $(CFLAGS) -o $@ $(myechoSF)
sortLines:
	gcc -ggdb3 -Wall -Werror -std=gnu99 -pedantic -o $@ $(sortLines)
myShell: $(myShellSF)
	g++ $(CFLAGS) -o $@ $(myShellSF)
clean:
	rm -f $(PROGRAM) *~
