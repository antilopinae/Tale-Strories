val kotlinVersion: String by project

group = "adapters"
version = "unspecified"

repositories {
    mavenCentral()
}

dependencies {
    implementation(project(":adapters:grpc"))
}