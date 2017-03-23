vecutil is a simple tool to compare vectors written in UG4's .vec format.
If you use it you may have to invest a little debugging for your special case.


BUILDING:
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
