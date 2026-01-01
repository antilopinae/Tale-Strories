# ЭТАП 1: Сборка JAR
FROM gradle:8.5-jdk21 AS build
WORKDIR /home/gradle/src

# Копируем весь корень (чтобы были доступны Api/proto и Backend/)
COPY --chown=gradle:gradle . .

# Переходим именно в папку бэкенда, где лежит gradlew
WORKDIR /home/gradle/src/Backend

# Даем права на выполнение (важно для Mac/Linux)
RUN chmod +x gradlew

# Ограничиваем память Gradle, чтобы не было ошибки "daemon disappeared"
ENV GRADLE_OPTS="-Xmx1536m -Dorg.gradle.daemon=false"

# Собираем JAR. Путь к проекту внутри Backend теперь просто :session-service
RUN ./gradlew :session-service:bootJar --no-daemon

# ЭТАП 2: Рантайм
FROM eclipse-temurin:21-jre
WORKDIR /app

# Копируем JAR из папки билда (путь изменился, так как мы собирали внутри /Backend)
COPY --from=build /home/gradle/src/Backend/session-service/build/libs/*.jar app.jar

EXPOSE 9090

ENTRYPOINT ["java", "-jar", "app.jar", "--spring.profiles.active=docker"]