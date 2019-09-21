#include "application.h"

#include "stdlib.h"

int main(int argc, char* argv[])
{
	/**
	 *	Test initialization. 
	 */

	application *app = malloc(sizeof(application));
	run(app);
}
