dist:
	rm -rf dist
	mkdir dist
	sign/setup.sh
	make -C tools
	sudo docker compose build
	sudo docker tag verified-toolbox-web verified-toolbox
	sudo docker save verified-toolbox > dist/verified-toolbox.tar
	cp docker-compose.yml start.sh dist
	sed -i 's/build: \./image: verified-toolbox/' dist/docker-compose.yml
	cd .. && zip -DXr verified-toolbox/dist/verified-toolbox.zip verified-toolbox/{app,docker-compose.yml,Dockerfile,entrypoint.sh,flag,readflag.c,.dockerignore} -x target
	cd dist && tar czf misc-verified-toolbox.tar.gz ./*

.PHONY: dist
