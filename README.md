## Findin C

Findin now for CLI usage on Windows, written 100% in C.

It's usage is pretty straight-forward. Set the program to your PATH and call it from the command line. It only takes two arguments:
- The *file extension*. It should be in the following format: ".c", ".cs", ".py";
- The *search content*. If it contains spaces, surround it with '' or "".

The program is extremely small (currently 14kb) and consumes very little memory. The most memory it has consumed in my tests (searching in a directory with almost 7GB in size, mostly source-code files) was something close to **996 kb**. There are many things related to optimization that can be done in the program to make it faster, so expect new releases.

If you find any problems with the program or want to change something for the better in the code, feel free to do so.

--------

[Click here](https://github.com/JustAn0therDev/Findin) if you want to check out the original version written in C# for Windows.
