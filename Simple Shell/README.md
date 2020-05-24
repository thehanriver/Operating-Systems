# Simple Linux Shell in C
## Description

The plan is to make a simple shell that's able to execute commands, pipe output commands to other commands, carry out commands in the background, and redirect some input/output stuff.

## Status

Some commands such as ls work while other operations such as piping and commands in the background doesn't yet. I plan to work on it a little bit more in the future for completion.

## Problems

For this project, I ran into alot of problems. Because of this I had a lot of trouble trying to implement the shell. 
It is able to do basic commands such as ls, cat, etc.,but when meta-characters were involved, execvp was always -1.
In doing so, I had to create multiple .c files and start over again multiple times. To no prevail, I was unable to 
fix the problem. I believe this was either due to my piping/fork or storing and parsing the user commands so that
execvp can be executed. Either way this can pass as a VERY simple shell but not a simple shell. I used tokens so that
execvp could work. I tried multiple ways such as using a for loop to store the command line without spaces or a while loop
but for some reason, execvp would always return -1 even though arguments[0] (where I stored the first argument for error testing)
print ls. I also tried to think of multiple ways to do it since some of the ways I tried had errors some didn't. In doing so,
it took alot of time and became a mess. This led to me not completely finishing in time since I couldn't figure out what was going on.
I had to rewrite a simple one and that is what I submitted.
