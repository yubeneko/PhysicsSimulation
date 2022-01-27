#include "Core.h"

int main ()
{
	Core core;
	bool success = core.Initialize();
	if (success)
	{
		core.RunLoop();
	}
	core.Shutdown();

	return 0;
}