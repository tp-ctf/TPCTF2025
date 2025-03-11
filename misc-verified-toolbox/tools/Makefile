all: greeting.jar arithmetic.jar
	mkdir -p ../app/src/main/resources/static/toolbox
	cp $^ ../app/src/main/resources/static/toolbox/

greeting.jar: greeting/hello.jar greeting/nihao.jar greeting/konnichiwa.jar
	cd greeting && jar cf0 ../$@ $(notdir $^)

arithmetic.jar: arithmetic/add.jar arithmetic/subtract.jar arithmetic/multiply.jar arithmetic/divide.jar
	cd arithmetic && jar cf0 ../$@ $(notdir $^)

%.jar: %/Tool.class ../sign/keystore.jks
	cd $* && jar cf ../$(notdir $@) Tool.class
	../sign/sign.sh $@

%/Tool.class: %/Tool.java
	javac $<

.PHONY: clean
clean:
	rm -f *.jar **/*.jar **/*.class
