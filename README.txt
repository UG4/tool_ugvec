vecutil is a simple tool to compare vectors written in UG4's .vec format.
If you use it you may have to invest a little debugging for your special case.

One word on vectors with multiple components. There are two main distinctions:
1:	(unblocked) Your vector contains multiple components stored as individual
	scalar values. This is the case if your .vec file contains the same position
	data multiple times (as many times as there are components).
	In this case you'll have to specify the number of components through the flag
	'-numComponents'.

2:	(blocked) Your vector contains multiple components stored as blocks. In this
	case your .vec file contains each position only once. For each connection,
	several values are stored inside a '[ ... ]' block. In this case the components
	are extracted automatically. Please do not specify '-numComponents' in this case.



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
