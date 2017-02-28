vecutil is a simple tool to compare vectors written in UG4's .vec format.
If you use it you may have to invest a little debugging for your special case.


BUILDING:
From the source directory:
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
To create a debug build, proceed as above use
	'cmake -DCMAKE_BUILD_TYPE=Debug ..'
instead of
	'cmake -DCMAKE_BUILD_TYPE=Release ..'
