#include "stdlib.h"
#include "stdio.h"

#include "application.h"

int main(int argc, char* argv[])
{
	/**
	 *	Test initialization. 
	 */

	application *app = malloc(sizeof(application));
	run(app);

	return 0;
}
