#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>


static int
unicom_modevent(module_t mod __unused, int event, void *arg __unused)
{
	int error = 0;
	switch (event) {
	case MOD_LOAD:
		uprintf("Hello, world!\n");
		break;
	case MOD_UNLOAD:
		uprintf("Good-bye, cruel world!\n");
		break;
	default:
		error = EOPNOTSUPP;
		break;
	}
	return (error);
}


static moduledata_t unicom_mod = {
	"unicom",
	unicom_modevent,
	NULL
};


DECLARE_MODULE(unicom, unicom_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
