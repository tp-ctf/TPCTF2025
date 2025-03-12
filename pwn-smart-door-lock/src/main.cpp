#include "door_lock.h"

int main(int argc, char *argv[])
{
	class mqtt_lock *lockconv;
	int rc;

	mosqpp::lib_init();
    lockconv = new mqtt_lock("tpctf_smart_door_lock", "127.0.0.1", 8883);

	lockconv->loop_forever();

	mosqpp::lib_cleanup();

	return 0;
}

