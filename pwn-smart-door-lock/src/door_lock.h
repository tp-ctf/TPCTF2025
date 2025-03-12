#ifndef DOOR_LOCL_H
#define DOOR_LOCL_H
#include <mosquittopp.h>
#include <string>


class mqtt_lock : public mosqpp::mosquittopp
{
	struct fingers {
		unsigned int finger[20];
		fingers* next;
		unsigned int finger_id;
		unsigned int retry_count;
	};
	struct lock_status {
		bool lock;
		std::string timestamp;
	};
	
	public:
		mqtt_lock(const char *id, const char *host, int port);
		~mqtt_lock();

		void on_connect(int rc);
		void on_disconnect(int rc);
		void on_message(const struct mosquitto_message *message);
		void on_publish(int mid);
		void on_unsubscribe(int mid);
		void on_subscribe(int mid, int qos_count, const int *granted_qos);
		
	private:
		fingers *finger_list;
		lock_status lock_status;
		FILE *logger;
		unsigned int max_finger_id;
		char log_file[32];
		char* session_id;
		char* auth_token;

		bool add_finger(char* finger_str);
		bool edit_finger(fingers* finger,char* finger_str);
		bool remove_finger(unsigned int finger_id);
		bool check_finger(fingers* finger,char* finger_str);
		bool download_log();
		bool clear_log();
		bool log(const char* str,...);
		bool lock_door();
		bool unlock_door();
};

#endif
