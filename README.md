Creepy Crawlies - An experiment in procedural animation

What if you could design a character, and then watch as it learns to move all by
itself?

* [Watch the demo](https://youtu.be/_v9_hTSOHN4)
* [Download the executable (Windows)](https://pedroluchini.github.io/creepycrawlies/CreepyCrawlies_bin.zip)

In this program, I experimented with procedural animation techniques to see how
far I could push them. It starts with a creature editor where you can place
bones and muscles on a virtual creature; then, when you're satisfied with your
design, you just tell the program to animate the creature and watch as it starts
walking!

I used physics simulation to represent the creature's body, and genetic
algorithms to develop the animation. If you're interested in learning how this
program works, I wrote [a paper](https://pedroluchini.github.io/creepycrawlies/PedroLuchini_ProceduralAnimation.pdf)
about it.

Please keep in mind that this is an experiment/prototype, and it has some bugs.

## BUILDING THE PROGRAM

This package includes Visual Studio 2003 solution/project files, as well as all
necessary dependencies (libraries and headers). Building the program should be
as easy as opening CreepyCrawlies.sln and hitting the "Build" button.

The executable is generated in the output folder. Both Debug and Release
versions are generated there. 

## ACKNOWLEDGEMENTS

This package includes parts of Matthew Wall's genetic algorithms library, GALib.
The full library can be found at the following address:

http://lancet.mit.edu/ga/

Also included are the binaries of Haaf's Game Engine, which is available here:

http://hge.relishgames.com
