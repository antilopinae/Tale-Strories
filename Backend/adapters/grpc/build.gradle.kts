import com.google.protobuf.gradle.*
import org.gradle.internal.impldep.org.junit.platform.engine.support.descriptor.DirectorySource

val grpcKotlinVersion: String by project
val grpcVersion: String by project
val protobufVersion: String by project
val kotlinxCoroutinesVersion: String by project

plugins {
    kotlin("jvm")
    id("com.google.protobuf")
}

sourceSets {
    main {
        proto {
            srcDir(File(projectDir, "../../../Api/proto"))
        }
    }
}

dependencies {
    api("io.grpc:grpc-kotlin-stub:$grpcKotlinVersion")
    api("io.grpc:grpc-stub:$grpcVersion")
    api("io.grpc:grpc-protobuf:$grpcVersion")
    api("com.google.protobuf:protobuf-kotlin:$protobufVersion")

    implementation("io.grpc:grpc-netty-shaded:$grpcVersion")
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:$kotlinxCoroutinesVersion")
}

protobuf {
    protoc {
        artifact = "com.google.protobuf:protoc:$protobufVersion"
    }
    plugins {
        id("grpc") { artifact = "io.grpc:protoc-gen-grpc-java:$grpcVersion" }
        id("grpckt") { artifact = "io.grpc:protoc-gen-grpc-kotlin:$grpcKotlinVersion:jdk8@jar" }
    }
    generateProtoTasks {
        all().forEach {
            it.plugins {
                id("grpc")
                id("grpckt")
            }
        }
    }
}