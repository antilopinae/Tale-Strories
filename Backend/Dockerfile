FROM gradle:8.5-jdk21 AS build
WORKDIR /home/gradle/src
COPY --chown=gradle:gradle . .

RUN ./gradlew :session-service:bootJar --no-daemon

FROM eclipse-temurin:21-jre
WORKDIR /app

COPY --from=build /home/gradle/src/session-service/build/libs/*.jar app.jar

EXPOSE 9090

ENTRYPOINT ["java", "-jar", "app.jar", "--spring.profiles.active=docker"]