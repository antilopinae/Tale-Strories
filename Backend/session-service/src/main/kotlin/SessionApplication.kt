package com.game.session

import com.grpc.*
import io.grpc.*
import io.grpc.stub.StreamObserver
import net.devh.boot.grpc.server.interceptor.GrpcGlobalServerInterceptor
import net.devh.boot.grpc.server.service.GrpcService
import org.springframework.beans.factory.annotation.Value
import org.springframework.boot.autoconfigure.SpringBootApplication
import org.springframework.boot.runApplication
import org.springframework.kafka.core.KafkaTemplate
import org.springframework.stereotype.Component
import java.util.*
import com.google.api.client.googleapis.auth.oauth2.GoogleIdTokenVerifier
import com.google.api.client.http.javanet.NetHttpTransport
import com.google.api.client.json.gson.GsonFactory
import com.auth0.jwt.JWT
import com.auth0.jwt.algorithms.Algorithm
import java.util.Date

@SpringBootApplication
class SessionApplication

fun main(args: Array<String>) {
    runApplication<SessionApplication>(*args)
}

// Контекст для передачи ID игрока между интерцептором и сервисами
object SecurityContext {
    val PLAYER_ID_KEY: Context.Key<String> = Context.key("playerId")
}

@GrpcService
class AuthGrpcService(
    @Value("\${game.auth.google-client-id}") private val googleClientId: String,
    @Value("\${game.auth.jwt-secret}") private val jwtSecret: String
) : AuthServiceGrpc.AuthServiceImplBase() {

    private val verifier = GoogleIdTokenVerifier.Builder(NetHttpTransport(), GsonFactory())
        .setAudience(listOf(googleClientId))
        .build()

    override fun authenticateWithGoogle(request: GoogleAuthRequest, responseObserver: StreamObserver<AuthResponse>) {
        try {
            // 1. Проверяем Google ID Token
            val idToken = verifier.verify(request.idToken)

            if (idToken != null) {
                val googleUserId = idToken.payload.subject
                val email = idToken.payload.email

                // Формируем внутренний ID игрока
                val playerId = "player_${googleUserId.take(10)}"

                // 2. Генерируем наш игровой JWT
                val expirationTime = System.currentTimeMillis() + 3600 * 1000 // 1 час
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
                println("✅ Auth: Игрок $email авторизован. Выдан JWT.")
            } else {
                responseObserver.onError(Status.UNAUTHENTICATED.withDescription("Invalid Google Token").asException())
            }
        } catch (e: Exception) {
            println("❌ Auth Error: ${e.message}")
            responseObserver.onError(Status.INTERNAL.withDescription("Server error during auth").asException())
        }
    }
}

@GrpcGlobalServerInterceptor
class JwtInterceptor(
    @Value("\${game.auth.jwt-secret}") private val jwtSecret: String
) : ServerInterceptor {

    override fun <ReqT : Any, RespT : Any> interceptCall(
        call: ServerCall<ReqT, RespT>,
        headers: Metadata,
        next: ServerCallHandler<ReqT, RespT>
    ): ServerCall.Listener<ReqT> {

        // Разрешаем вызов AuthService без токена
        if (call.methodDescriptor.serviceName != null && call.methodDescriptor.serviceName!!.contains("AuthService")) {
            return next.startCall(call, headers)
        }

        val authHeader = headers.get(Metadata.Key.of("authorization", Metadata.ASCII_STRING_MARSHALLER))

        if (authHeader != null && authHeader.startsWith("Bearer ")) {
            val token = authHeader.substring(7)
            try {
                // Проверяем подпись нашего JWT
                val decodedJWT = JWT.require(Algorithm.HMAC256(jwtSecret)).build().verify(token)
                val playerId = decodedJWT.subject

                // Кладем playerId в контекст запроса
                val context = Context.current().withValue(SecurityContext.PLAYER_ID_KEY, playerId)
                return Contexts.interceptCall(context, call, headers, next)
            } catch (e: Exception) {
                call.close(Status.UNAUTHENTICATED.withDescription("Session expired"), Metadata())
            }
        } else {
            call.close(Status.UNAUTHENTICATED.withDescription("No authorization provided"), Metadata())
        }

        return object : ServerCall.Listener<ReqT>() {}
    }
}

@GrpcService
class GameGrpcService(
    private val kafkaTemplate: KafkaTemplate<String, String>
) : GameServiceGrpc.GameServiceImplBase() {

    override fun joinSession(request: JoinRequest, responseObserver: StreamObserver<JoinResponse>) {
        // Берем ID из защищенного контекста
        val playerId = SecurityContext.PLAYER_ID_KEY.get() ?: "anonymous"
        val sessionId = UUID.randomUUID().toString()

        kafkaTemplate.send("session-events", sessionId, "PLAYER_JOINED:$playerId")

        val response = JoinResponse.newBuilder()
            .setSessionId(sessionId)
            .setStatus("SUCCESS")
            .build()

        responseObserver.onNext(response)
        responseObserver.onCompleted()
        println("📡 Game: Игрок $playerId зашел в сессию $sessionId")
    }

    override fun getMapLayout(request: MapRequest, responseObserver: StreamObserver<MapLayout>) {
        val layout = MapLayout.newBuilder()
            .setTimerSeconds(600)
            .addRooms(SubLocation.newBuilder().setId("Spawn").setX(0f).setY(0f).build())
            .build()
        responseObserver.onNext(layout)
        responseObserver.onCompleted()
    }
}