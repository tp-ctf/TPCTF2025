FROM maven:3.9-eclipse-temurin-21 AS build

RUN apt-get update
RUN apt-get install -y gcc

WORKDIR /src
COPY readflag.c .
RUN gcc readflag.c -o readflag

COPY app /app
WORKDIR /app
RUN mvn package

FROM eclipse-temurin:21-jre

COPY --chmod=0400 flag /
COPY --chmod=0500 entrypoint.sh /
COPY --chmod=4555 --from=build /src/readflag /
COPY --chmod=0444 --from=build /app/target/*.jar /app.jar

RUN groupadd -r ctf && useradd -r -g ctf ctf

ENTRYPOINT ["/entrypoint.sh"]
