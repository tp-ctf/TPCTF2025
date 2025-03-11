package org.tpctf.verified_toolbox;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.loader.net.protocol.Handlers;

@SpringBootApplication
public class VerifiedToolboxApplication {
	public static void main(String[] args) {
        Handlers.register();
		SpringApplication.run(VerifiedToolboxApplication.class, args);
	}
}
