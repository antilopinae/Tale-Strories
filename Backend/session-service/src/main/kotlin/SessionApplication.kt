package com.game.session

import com.grpc.*
import io.grpc.*
import io.grpc.stub.StreamObserver
import net.devh.boot.grpc.server.interceptor.GrpcGlobalServerInterceptor
import net.devh.boot.grpc.server.service.GrpcService
import org.springframework.beans.factory.annotation.Value
import org.springframework.boot.autoconfigure.SpringBootApplication
import org.springframework.boot.runApplication
import org.springframework.stereotype.Service
import com.google.api.client.googleapis.auth.oauth2.GoogleAuthorizationCodeTokenRequest
import com.google.api.client.http.javanet.NetHttpTransport
import com.google.api.client.json.gson.GsonFactory
import com.auth0.jwt.JWT
import com.auth0.jwt.algorithms.Algorithm
import com.github.dockerjava.api.DockerClient
import com.github.dockerjava.core.DefaultDockerClientConfig
import com.github.dockerjava.httpclient5.ApacheDockerHttpClient
import com.github.dockerjava.core.DockerClientImpl
import com.github.dockerjava.api.model.HostConfig
import com.github.dockerjava.api.model.PortBinding
import java.util.*
import java.util.concurrent.ConcurrentHashMap

@SpringBootApplication
class SessionApplication

fun main(args: Array<String>) {
    runApplication<SessionApplication>(*args)
}

object SecurityContext {
    val PLAYER_ID_KEY: Context.Key<String> = Context.key("playerId")
}

// --- СЕРВИС УПРАВЛЕНИЯ DOCKER ---
@Service
class DockerOrchestrator(
    @Value("\${GAME_SERVER_IMAGE:tale-stories-cpp-server:latest}")
    private val imageGameServer: String
) {
    // Инициализируем клиент один раз
    private val dockerClient: DockerClient by lazy {
        val config = DefaultDockerClientConfig.createDefaultConfigBuilder()
            // Если переменная окружения DOCKER_HOST пуста, используем стандартный путь
            .withDockerHost(System.getenv("DOCKER_HOST") ?: "unix:///var/run/docker.sock")
            .build()

        val httpClient = ApacheDockerHttpClient.Builder()
            .dockerHost(config.dockerHost)
            .sslConfig(config.sslConfig)
            .maxConnections(100)
            .build()

        DockerClientImpl.getInstance(config, httpClient)
    }

    fun spawnGameServer(roomId: String): Int {
        // Проверяем, существует ли образ, прежде чем запускать
        // (Опционально: можно добавить dockerClient.pullImageCmd)

        val port = 55000 + Random().nextInt(1000)

        try {
            val container = dockerClient.createContainerCmd(imageGameServer)
                .withName("room_${roomId}_${System.currentTimeMillis()}")
                .withHostConfig(
                    HostConfig.newHostConfig()
                        .withPortBindings(PortBinding.parse("$port:9000"))
                        .withAutoRemove(true) // Контейнер сам удалится после выключения
                )
                .exec()

            dockerClient.startContainerCmd(container.id).exec()
            println("🚀 Docker: Started server for room $roomId on port $port")
            return port
        } catch (e: Exception) {
            println("❌ Docker Error: ${e.message}")
            throw e
        }
    }
}

// --- LOBBY SERVICE (Управление комнатами) ---
@GrpcService
class LobbyGrpcService(private val orchestrator: DockerOrchestrator) : LobbyServiceGrpc.LobbyServiceImplBase() {

    // Храним соответствие RoomID -> Port
    private val roomServers = ConcurrentHashMap<String, Int>()
    private val roomName = UUID.randomUUID().toString()

    override fun joinRoom(request: JoinRoomRequest, responseObserver: StreamObserver<JoinRoomResponse>) {
        val playerId = SecurityContext.PLAYER_ID_KEY.get() ?: "unknown"
        val roomName = roomName

        try {
            // Если для этой комнаты еще нет сервера — создаем
//            val port = roomServers.getOrPut(roomName) {
//                orchestrator.spawnGameServer(roomName)
//            }

            val response = JoinRoomResponse.newBuilder()
                .setStatus(ResponseStatus.OK)
                .setMessage("Server is ready")
                .setRoomSessionId(UUID.randomUUID().toString())
                .setServerInfo(
                    ServerInfo.newBuilder()
//                        .setAddress("127.0.0.1:$port") // Локально. В проде тут будет внешний IP
                        .setAddress("host.docker.internal:9000")
                        .setServerVersion("1.0.0")
                        .build()
                )
                .build()

            responseObserver.onNext(response)
            responseObserver.onCompleted()
            println("🏠 Lobby: Игрок $playerId направлен в комнату $roomName на порт 9000")

        } catch (e: Exception) {
            responseObserver.onNext(
                JoinRoomResponse.newBuilder()
                    .setStatus(ResponseStatus.ERROR)
                    .setMessage("Failed to spawn server: ${e.message}")
                    .build()
            )
            responseObserver.onCompleted()
        }
    }
}

// --- AUTH SERVICE (Code Flow) ---
@GrpcService
class AuthGrpcService(
    @Value("\${game.auth.google-client-id}") private val googleClientId: String,
    @Value("\${game.auth.google-client-secret}") private val googleClientSecret: String,
    @Value("\${game.auth.jwt-secret}") private val jwtSecret: String
) : AuthServiceGrpc.AuthServiceImplBase() {

    override fun authenticateWithGoogle(request: GoogleAuthRequest, responseObserver: StreamObserver<AuthResponse>) {
        try {
            // Обмениваем AUTH CODE на токены (Safe Server-side flow)
            val tokenResponse = GoogleAuthorizationCodeTokenRequest(
                NetHttpTransport(),
                GsonFactory(),
                "https://oauth2.googleapis.com/token",
                googleClientId,
                googleClientSecret,
                request.authCode,
                request.redirectUri
            ).execute()

            val idToken = tokenResponse.parseIdToken()
            val googleUserId = idToken.payload.subject
            val email = idToken.payload.email

            val playerId = "player_${googleUserId.take(10)}"
            val expirationTime = System.currentTimeMillis() + 3600 * 1000

            val token = JWT.create()
                .withSubject(playerId)
                .withClaim("email", email)
                .withExpiresAt(Date(expirationTime))
                .sign(Algorithm.HMAC256(jwtSecret))

            val response = AuthResponse.newBuilder()
                .setAccessToken(token)
                .setPlayerId(playerId)
                .setExpiresAt(expirationTime)
                .build()

            responseObserver.onNext(response)
            responseObserver.onCompleted()
            println("✅ Auth: Игрок $email успешно вошел через Code Flow")

        } catch (e: Exception) {
            println("❌ Auth Error: ${e.message}")
            responseObserver.onError(Status.UNAUTHENTICATED.withDescription("Google Auth Failed").asException())
        }
    }
}

// --- INTERCEPTOR (Защита лобби) ---
@GrpcGlobalServerInterceptor
class JwtInterceptor(
    @Value("\${game.auth.jwt-secret}") private val jwtSecret: String
) : ServerInterceptor {

    override fun <ReqT : Any, RespT : Any> interceptCall(
        call: ServerCall<ReqT, RespT>,
        headers: Metadata,
        next: ServerCallHandler<ReqT, RespT>
    ): ServerCall.Listener<ReqT> {

        // Пропускаем только AuthService
        if (call.methodDescriptor.serviceName != null && call.methodDescriptor.serviceName!!.contains("AuthService")) {
            return next.startCall(call, headers)
        }

        val authHeader = headers.get(Metadata.Key.of("authorization", Metadata.ASCII_STRING_MARSHALLER))

        if (authHeader != null && authHeader.startsWith("Bearer ")) {
            val token = authHeader.substring(7)
            return try {
                val decodedJWT = JWT.require(Algorithm.HMAC256(jwtSecret)).build().verify(token)
                val context = Context.current().withValue(SecurityContext.PLAYER_ID_KEY, decodedJWT.subject)
                Contexts.interceptCall(context, call, headers, next)
            } catch (e: Exception) {
                call.close(Status.UNAUTHENTICATED.withDescription("Invalid JWT"), Metadata())
                object : ServerCall.Listener<ReqT>() {}
            }
        }

        call.close(Status.UNAUTHENTICATED.withDescription("No token provided"), Metadata())
        return object : ServerCall.Listener<ReqT>() {}
    }
}