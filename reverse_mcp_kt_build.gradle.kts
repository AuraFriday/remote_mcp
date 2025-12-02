// Gradle build script for Kotlin Reverse MCP (JVM version)
// This builds a fat JAR with all dependencies included
// "signature": "EYᎠⅼΡϹN𝟚𝟟W×GģԝƍᖴÐŧΥꓚᏴօҳUĐƐĐϜƖm9ΒΤⲢⅠrрQցɡģȠpAꓐ4КƱꓐƶʋᎪꙄωᴠ6𐓒𝟟Sꓐ0ɯoLꓴᴅ𝟙rƳꙅᎻj𝟚ƛƨЈᗞȷ5Ƭһ𝟣ҳ𝟚МďΤᗷƿAᗷⅼոꓚƲꓴ𝟣ϜpꓚꓬÞyųοꓜꓦⲦ𝛢",
// "signdate": "2025-11-21T21:27:07.225Z",

plugins {
    kotlin("jvm") version "2.2.21"
    application
}

repositories {
    mavenCentral()
}

// Set Java compatibility (use Java 25 which is installed)
java {
    sourceCompatibility = JavaVersion.VERSION_11
    targetCompatibility = JavaVersion.VERSION_11
}

kotlin {
    jvmToolchain {
        languageVersion.set(JavaLanguageVersion.of(25))
    }
    
    // Explicit source directory configuration
    sourceSets {
        main {
            kotlin.srcDirs("reverse_mcp_kt_src/main/kotlin")
        }
    }
}

application {
    mainClass.set("Reverse_mcpKt")
}

dependencies {
    implementation(kotlin("stdlib"))
}

// Create a fat JAR with all dependencies
tasks.jar {
    manifest {
        attributes["Main-Class"] = "Reverse_mcpKt"
    }
    
    // Include all dependencies in the JAR
    duplicatesStrategy = DuplicatesStrategy.EXCLUDE
    from(configurations.runtimeClasspath.get().map { 
        if (it.isDirectory) it else zipTree(it) 
    })
    
    archiveFileName.set("reverse_mcp_kt.jar")
}

// Configure Kotlin compilation (using modern compilerOptions DSL)
tasks.withType<org.jetbrains.kotlin.gradle.tasks.KotlinCompile> {
    compilerOptions {
        jvmTarget.set(org.jetbrains.kotlin.gradle.dsl.JvmTarget.JVM_11)
        freeCompilerArgs.add("-Xjvm-default=all")
    }
}


