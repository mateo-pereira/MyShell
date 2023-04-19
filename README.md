# MyShell
Fully functioning command-line shell using POSIX API in C that uses interactive and batch mode similar to a UNIX/Linux operating system

Test Plan:
	Our program is redone simple command-line shell, similar to bash or zsh. We have implemented
an interactive mode capable of reading from an input file and executing the commands written
in it.

-------------------- 

Reading Inputs

In interactive mode, our program read user input and used a buffer to safely read and write every word
into an input string, which was then transferred to a 2d array for easier word by word access. Then,
a Linked List struct was implemented that contained the executed file name, input and output files, arguments,
and more. Each node was a different command that was part of the line that the user inputted originally.



Reading and running commands

Using this struct named "Process", our program executed commands in separate functions that took the input
of a node in "Process".

Extensions:
cd "home"
---------------------

Tested Inputs 

No redirect
No pipe
No batchmode
No wildcards

Interactive mode should read:
// testing spacing and parsing

echo         foo bar     baz
// prints: "foo bar baz"

echo 

// testing regular commands
echo boo
touch test.txt
cat text.txt
mkdir testdir
pwd
cd testdir 
cd ..
cd // will do home extension
// from home directory
cd cs214/pa2
exit // will exit
