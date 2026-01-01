package com.game.client

import io.grpc.ManagedChannelBuilder

fun main() {
    println("Запуск тестового клиента TaleStories...")

//    val channel = ManagedChannelBuilder.forAddress("localhost", 9090)
//        .usePlaintext()
//        .build()
//
//    val stub = GameServiceGrpc.newBlockingStub(channel)
//
//    val joinResponse = stub.joinSession(JoinRequest.newBuilder().setPlayerId("Player_1").build())
//    val sid = joinResponse.sessionId
//    println("Получен SessionID: $sid")
//
//    val map = stub.getMapLayout(MapRequest.newBuilder().setSessionId(sid).build())
//    println("\nЗагружена карта:")
//    map.roomsList.forEach { room ->
//        println(" - Комната ${room.id}: Позиция (${room.x}, ${room.y}), Размер ${room.width}x${room.height}")
//    }
//
//    channel.shutdown()
}