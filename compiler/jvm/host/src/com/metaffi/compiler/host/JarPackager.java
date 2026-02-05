package com.metaffi.compiler.host;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.stream.Collectors;

public class JarPackager {

    public Path compileAndPackage(Path sourceRoot, Path outputDir, String outputName,
                                  List<String> classpathEntries, String javaTarget, boolean debugInfo) {
        if (sourceRoot == null) {
            throw new CompilerException("sourceRoot is required");
        }
        if (outputDir == null) {
            throw new CompilerException("outputDir is required");
        }
        if (outputName == null || outputName.isEmpty()) {
            throw new CompilerException("outputName is required");
        }

        try {
            Files.createDirectories(outputDir);
        } catch (IOException e) {
            throw new CompilerException("Failed to create output directory: " + outputDir, e);
        }

        List<Path> javaFiles = findJavaFiles(sourceRoot);
        if (javaFiles.isEmpty()) {
            throw new CompilerException("No .java files found under " + sourceRoot);
        }

        Path classesDir;
        try {
            classesDir = Files.createTempDirectory(outputDir, "metaffi_jvm_classes_");
        } catch (IOException e) {
            throw new CompilerException("Failed to create classes output directory", e);
        }

        List<String> classpath = new ArrayList<>();
        classpath.add(resolveMetaFFIApiJar());
        if (classpathEntries != null) {
            classpath.addAll(classpathEntries);
        }

        String classpathStr = buildClasspath(classpath);

        List<String> javacCmd = new ArrayList<>();
        javacCmd.add("javac");
        String release = (javaTarget == null || javaTarget.isEmpty()) ? "11" : javaTarget;
        javacCmd.add("--release");
        javacCmd.add(release);
        if (debugInfo) {
            javacCmd.add("-g");
        }
        if (!classpathStr.isEmpty()) {
            javacCmd.add("-cp");
            javacCmd.add(classpathStr);
        }
        javacCmd.add("-d");
        javacCmd.add(classesDir.toString());
        for (Path file : javaFiles) {
            javacCmd.add(file.toString());
        }

        runCommand(javacCmd, sourceRoot);

        Path jarPath = outputDir.resolve(outputName + ".jar");
        List<String> jarCmd = new ArrayList<>();
        jarCmd.add("jar");
        jarCmd.add("cf");
        jarCmd.add(jarPath.toString());
        jarCmd.add("-C");
        jarCmd.add(classesDir.toString());
        jarCmd.add(".");

        runCommand(jarCmd, sourceRoot);
        return jarPath;
    }

    private List<Path> findJavaFiles(Path sourceRoot) {
        if (!Files.exists(sourceRoot)) {
            return Collections.emptyList();
        }
        try {
            return Files.walk(sourceRoot)
                .filter(p -> p.toString().toLowerCase(Locale.ROOT).endsWith(".java"))
                .collect(Collectors.toList());
        } catch (IOException e) {
            throw new CompilerException("Failed to scan java files under " + sourceRoot, e);
        }
    }

    private String buildClasspath(List<String> entries) {
        if (entries == null || entries.isEmpty()) {
            return "";
        }
        List<String> filtered = entries.stream()
            .filter(e -> e != null && !e.isEmpty())
            .collect(Collectors.toList());
        return String.join(System.getProperty("path.separator"), filtered);
    }

    private String resolveMetaFFIApiJar() {
        String metaffiHome = System.getenv("METAFFI_HOME");
        String metaffiSourceRoot = System.getenv("METAFFI_SOURCE_ROOT");

        List<Path> candidates = new ArrayList<>();
        if (metaffiHome != null && !metaffiHome.isEmpty()) {
            candidates.add(Paths.get(metaffiHome, "jvm", "metaffi.api.jar"));
            candidates.add(Paths.get(metaffiHome, "sdk", "api", "jvm", "metaffi.api.jar"));
        }
        if (metaffiSourceRoot != null && !metaffiSourceRoot.isEmpty()) {
            candidates.add(Paths.get(metaffiSourceRoot, "sdk", "api", "jvm", "metaffi.api.jar"));
        }

        for (Path p : candidates) {
            if (Files.exists(p)) {
                return p.toString();
            }
        }

        throw new CompilerException("Could not locate metaffi.api.jar. Set METAFFI_HOME or METAFFI_SOURCE_ROOT and build the JVM API.");
    }

    private void runCommand(List<String> command, Path workingDir) {
        ProcessBuilder pb = new ProcessBuilder(command);
        if (workingDir != null) {
            pb.directory(workingDir.toFile());
        }
        pb.redirectErrorStream(true);

        try {
            Process proc = pb.start();
            String output;
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(proc.getInputStream()))) {
                output = reader.lines().collect(Collectors.joining(System.lineSeparator()));
            }
            int exit = proc.waitFor();
            if (exit != 0) {
                throw new CompilerException("Command failed: " + String.join(" ", command) + System.lineSeparator() + output);
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new CompilerException("Command interrupted: " + String.join(" ", command), e);
        } catch (IOException e) {
            throw new CompilerException("Failed to run command: " + String.join(" ", command), e);
        }
    }
}
