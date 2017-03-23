# vecutil #

**vecutil** is a simple tool to compare and analyse vectors written in ug4's .vec format.

## INSTALLATION: ##
Install vecutil with ug4's package manager [ughub](https://github.com/UG4/ughub).:
	'ughub install vecutil'

Or simply clone this repository to any place on your harddrive.


## BUILDING: ##
As part of UG4 (e.g. after installing vecutil through 'ughub install vecutil'):
Go to your ug4 build directory and execute:
	'cmake -Dvecutil=ON .''
	'make'
The resulting vecutil executable will be written to ug4/bin


From the source directory as standalone application:
- create a 'build' directory:
	'mkdir build'
- call cmake from that directory:
	'cd build'
	'cmake -DCMAKE_BUILD_TYPE=Release ..'
- call 'make' and 'make install'. You may need root privileges for the second.
	'make && make install'
  or (on Ubuntu)
    'make && sudo make install'


DEBUGGING:
To create a debug build, proceed as above and use
	'cmake -DCMAKE_BUILD_TYPE=Debug .'
instead of
	'cmake -DCMAKE_BUILD_TYPE=Release .'
