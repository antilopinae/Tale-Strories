package com.game.session

import org.springframework.kafka.annotation.KafkaListener
import org.springframework.kafka.core.KafkaTemplate
import org.springframework.stereotype.Service

@Service
class GameEventListener(
    private val kafkaTemplate: KafkaTemplate<String, String>
) {
    @KafkaListener(topics = ["session-events"], groupId = "tale-stories-group")
    fun handleSessionEvent(message: String) {
        println("Kafka Consumer получил: $message")

        if (message.startsWith("PLAYER_JOINED")) {
            val playerId = message.split(":")[1]

            Thread {
                Thread.sleep(5000)
                println("Игрок $playerId погиб в бою! Отправляем combat-event...")
                kafkaTemplate.send("combat-events", playerId, "LOST_ALL_LOOT")
            }.start()
        }
    }

    @KafkaListener(topics = ["combat-events"], groupId = "analytics-group")
    fun handleCombat(message: String) {
        println("Аналитика: Зафиксирована потеря лута: $message")
    }
}