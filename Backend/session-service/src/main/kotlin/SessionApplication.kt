package com.game.session

import com.grpc.*
import io.grpc.stub.StreamObserver
import net.devh.boot.grpc.server.service.GrpcService
import org.springframework.boot.autoconfigure.SpringBootApplication
import org.springframework.boot.runApplication
import org.springframework.kafka.core.KafkaTemplate
import java.util.*

@SpringBootApplication
class SessionApplication

fun main(args: Array<String>) {
    runApplication<SessionApplication>(*args)
}

@GrpcService
class GameGrpcService(
    private val kafkaTemplate: KafkaTemplate<String, String>
) : GameServiceGrpc.GameServiceImplBase() {

    override fun joinSession(request: JoinRequest, responseObserver: StreamObserver<JoinResponse>) {
        val sessionId = UUID.randomUUID().toString()
        val playerId = request.playerId

        kafkaTemplate.send("session-events", sessionId, "PLAYER_JOINED:$playerId")

        val response = JoinResponse.newBuilder()
            .setSessionId(sessionId)
            .setStatus("SUCCESS")
            .build()

        responseObserver.onNext(response)
        responseObserver.onCompleted()
        println("📡 Kafka: Отправлено событие входа для $playerId")
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