Assignment 1
CPSC 441
Glenn Skelton
10041868

		
-------------------------------------- MANUAL ---------------------------------------

COMPILE: 

To compile this program use the following command or use the commands found in 
the MAKEFILE section below:
$ gcc -Wall -g proxy.c -o proxy

To run enter the following:
$ ./proxy <port number>


*************************************************************************************

CONFIGURE:

The testing for my proxy was completed on a virtual box version of a Ubuntu linux
distribution. I used a Mozilla Firefox browser with the caching turned off. To turn
the caching off properly in order for refresh to work the way expected I adjusted
the following settings:

	type "about:config" in the search bar
	search and turn to false - "browser.cache.disk.enable"
	search and turn to false - "browser.cache.memory.enable"
	
instructions from https://support.mozilla.org/en-US/questions/905902

This setting was used so that I did not have to clear my cache every time I wanted 
to reload the page with new mistakes.

For debugging mode, set the DEBUG macro to 1. To use in single page mode (run only
once before terminating) set the SINGLE_LOOP macro to 1.

To change the number of errors that show up in the returned page, there is a macro
called ERRORS that can be changed to adjust the number of errors you wish to see.
A note about errors is that I made the decision to allow errors to show up at any 
point in a returned page, including between any valid tag (this includes the title 
at the top of the web browser) but not violating a tag itself.

You may choose any port number available you like but it must be the same as the 
one in your browser cache settings. The IP address selected for this proxy is
127.0.0.1.

**************************************************************************************

MAKEFILE:

I have also included a simple makefile for compiling and running the project

To make the object file, type "make proxy.o"
To build the executable and object file, type "make all"
To run a default version with a proxy (12345) already set, type "make run" although
you may also run it as "./proxy <your chosen port>" instead 
To clean up all object and execeutable files, type "make clean" 

**************************************************************************************

NOTES:

My proxy is able to take an http request and parse the necessary information for 
creating a socket. It can receive as large a content as is necessary from the 
web server and display it through looping temporary buffers. It also can parse 
a return message from the server and determine based on the HTTP codes given, 
what the appropriate action is. In most cases I have decided to allow the browser
take care of the codes by, for instance, displaying the returned response for a 
404 error. The same goes for other types of errors such as the different redirects
since the browser takes care of this for me. However, if a valid 200 request comes
in, the proxy will parse the content and determine whether it is a valid file to 
apply errors to. Based on its type, it will apply the correct algorithm to 
sabotage the file for simple text or html. If the file is not a valid form to 
apply errors to, it will simply forward the content to the browser unaltered.

The testing for this proxy was completed on a virtual box distribution of 
Ubuntu and used Firefox as the browser of choice. I ran initial testing using the 
sample web pages provided as well as other simple pages that I knew. After that 
I scaled up my testing to include more complicated content that contained 
multiple types of data such as binary image files and mixes of html and CSS.
The only issue I ran into was with the web caching as described above and as such, 
I was never able to fully test a 304 code. Other than this though, my proxy appears
to have behaved the way I expected after multiple tests.I relied on heavy debugging
statements along the way to ensure that I was receiving the expected content. The only 
thing I was not sure about was with regards to memory allocation. I had tried to 
allocate memory to the various structs I had used however, I had run into issues 
with their proper use in this style of programming. I also ran Valgrind on my code
and realized that there were allocations and frees (equal number, no memory leaks)
occuring which lead me to believe that objects like the socket structs take care of 
allocating and deallocating their own memory when calling the various creation and 
closing functions available for them. My proxy does not work for HTTPS web pages.

I did not attempt to do the bonus challenges for reasons of time constraints 

----------------------------------------------------------------------------------------
