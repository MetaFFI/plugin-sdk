package com.metaffi.compiler.host;

import com.google.gson.Gson;
import com.metaffi.idl.entities.ArgDefinition;
import com.metaffi.idl.entities.ClassDefinition;
import com.metaffi.idl.entities.ConstructorDefinition;
import com.metaffi.idl.entities.FunctionDefinition;
import com.metaffi.idl.entities.GlobalDefinition;
import com.metaffi.idl.entities.IDLDefinition;
import com.metaffi.idl.entities.MethodDefinition;
import com.metaffi.idl.entities.ModuleDefinition;

import java.io.BufferedReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

public class HostCompiler {
    private final CompilerContext context;
    private final CodeGenerator codeGenerator;
    private final JarPackager jarPackager;

    public HostCompiler(CompilerContext context) {
        if (context == null) {
            throw new CompilerException("CompilerContext is required");
        }
        this.context = context;
        this.codeGenerator = new CodeGenerator(new TypeInfoGenerator());
        this.jarPackager = new JarPackager();
    }

    public void compile(IDLDefinition definition, String outputDir, String outputFilename, Map<String, String> hostOptions) {
        if (definition == null) {
            throw new CompilerException("IDLDefinition is required");
        }
        if (outputDir == null || outputDir.isEmpty()) {
            throw new CompilerException("outputDir is required");
        }
        if (outputFilename == null || outputFilename.isEmpty()) {
            throw new CompilerException("outputFilename is required");
        }

        definition.finalizeConstruction();
        validateDefinition(definition);

        String className = outputFilename + "_MetaFFIHost";
        requireJavaIdentifier(className, "outputFilename");

        Map<String, String> options = (hostOptions != null) ? hostOptions : new HashMap<>();
        String packageOverride = getStringOption(options, "package");
        boolean compileToJar = getBooleanOption(options, "compile_to_jar", false);
        boolean debugInfo = getBooleanOption(options, "debug_info", false);
        String javaTarget = getStringOption(options, "java_target");
        String additionalClasspath = getStringOption(options, "additional_classpath");

        Path outputPath = Paths.get(outputDir);
        try {
            Files.createDirectories(outputPath);
        } catch (IOException e) {
            throw new CompilerException("Failed to create output directory: " + outputDir, e);
        }

        List<String> classpathEntries = new ArrayList<>();
        if (additionalClasspath != null && !additionalClasspath.isEmpty()) {
            String separator = System.getProperty("path.separator");
            for (String entry : additionalClasspath.split(java.util.regex.Pattern.quote(separator))) {
                if (!entry.isEmpty()) {
                    classpathEntries.add(entry);
                }
            }
        }

        List<String> externalResources = new ArrayList<>();
        for (ModuleDefinition module : definition.getModules()) {
            if (module.getExternalResources() != null) {
                externalResources.addAll(module.getExternalResources());
            }
        }
        classpathEntries.addAll(externalResources);

        for (ModuleDefinition module : definition.getModules()) {
            String packageName = (packageOverride != null && !packageOverride.isEmpty()) ? packageOverride : module.getName();
            requireJavaQualifiedName(packageName, "package name");

            Path packageDir = outputPath.resolve(packageName.replace('.', java.io.File.separatorChar));
            try {
                Files.createDirectories(packageDir);
            } catch (IOException e) {
                throw new CompilerException("Failed to create package directory: " + packageDir, e);
            }

            String code = codeGenerator.generateModuleCode(module, definition, packageName, className);
            Path outputFile = packageDir.resolve(className + ".java");
            try {
                Files.writeString(outputFile, code);
            } catch (IOException e) {
                throw new CompilerException("Failed to write output file: " + outputFile, e);
            }
        }

        if (compileToJar) {
            jarPackager.compileAndPackage(outputPath, outputPath, outputFilename, classpathEntries, javaTarget, debugInfo);
        }
    }

    public static void compileIdlJson(String idlJsonPath, String outputDir, String outputFilename, Map<String, String> hostOptions) {
        if (idlJsonPath == null || idlJsonPath.isEmpty()) {
            throw new CompilerException("idlJsonPath is required");
        }
        CompilerContext context = CompilerContext.defaultContext();
        Gson gson = context.getGson();
        IDLDefinition definition;
        try (BufferedReader reader = Files.newBufferedReader(Paths.get(idlJsonPath))) {
            definition = gson.fromJson(reader, IDLDefinition.class);
        } catch (IOException e) {
            throw new CompilerException("Failed to read IDL JSON: " + idlJsonPath, e);
        }
        if (definition == null) {
            throw new CompilerException("Failed to parse IDL JSON: " + idlJsonPath);
        }
        definition.finalizeConstruction();

        HostCompiler compiler = new HostCompiler(context);
        compiler.compile(definition, outputDir, outputFilename, hostOptions);
    }

    private void validateDefinition(IDLDefinition definition) {
        if (isNullOrEmpty(definition.getTargetLanguage())) {
            throw new CompilerException("IDL definition missing target_language");
        }
        if (isNullOrEmpty(definition.getMetaffiGuestLib())) {
            throw new CompilerException("IDL definition missing metaffi_guest_lib");
        }
        if (definition.getModules() == null || definition.getModules().isEmpty()) {
            throw new CompilerException("IDL definition has no modules");
        }

        for (ModuleDefinition module : definition.getModules()) {
            if (isNullOrEmpty(module.getName())) {
                throw new CompilerException("Module missing name");
            }

            for (FunctionDefinition func : module.getFunctions()) {
                requireJavaIdentifier(func.getName(), "function name");
                requireEntityPath(func.getEntityPath(), "function", func.getName());
                validateArgs(func.getParameters(), "function parameters", func.getName());
                validateArgs(func.getReturnValues(), "function return values", func.getName());
            }

            for (ClassDefinition cls : module.getClasses()) {
                requireJavaIdentifier(cls.getName(), "class name");
                requireEntityPath(cls.getEntityPath(), "class", cls.getName());

                for (ConstructorDefinition ctor : cls.getConstructors()) {
                    requireJavaIdentifier(ctor.getName(), "constructor name");
                    requireEntityPath(ctor.getEntityPath(), "constructor", ctor.getName());
                    validateArgs(ctor.getParameters(), "constructor parameters", ctor.getName());
                    validateArgs(ctor.getReturnValues(), "constructor return values", ctor.getName());
                    if (ctor.getReturnValues() == null || ctor.getReturnValues().size() != 1) {
                        throw new CompilerException("Constructor " + ctor.getName() + " in class " + cls.getName() + " must return exactly one value");
                    }
                }

                if (cls.getRelease() != null) {
                    MethodDefinition release = cls.getRelease();
                    requireJavaIdentifier(release.getName(), "release name");
                    requireEntityPath(release.getEntityPath(), "release", release.getName());
                    if (release.isInstanceRequired() && (release.getParameters() == null || release.getParameters().isEmpty())) {
                        throw new CompilerException("Instance release method " + release.getName() + " missing handle parameter");
                    }
                    validateArgs(release.getParameters(), "release parameters", release.getName());
                    validateArgs(release.getReturnValues(), "release return values", release.getName());
                }

                for (MethodDefinition method : cls.getMethods()) {
                    requireJavaIdentifier(method.getName(), "method name");
                    requireEntityPath(method.getEntityPath(), "method", method.getName());
                    if (method.isInstanceRequired() && (method.getParameters() == null || method.getParameters().isEmpty())) {
                        throw new CompilerException("Instance method " + method.getName() + " missing handle parameter");
                    }
                    validateArgs(method.getParameters(), "method parameters", method.getName());
                    validateArgs(method.getReturnValues(), "method return values", method.getName());
                }

                for (com.metaffi.idl.entities.FieldDefinition field : cls.getFields()) {
                    requireJavaIdentifier(field.getName(), "field name");
                    if (field.getGetter() == null && field.getSetter() == null) {
                        throw new CompilerException("Field " + field.getName() + " has neither getter nor setter");
                    }
                    if (field.getGetter() != null) {
                        MethodDefinition getter = field.getGetter();
                        requireJavaIdentifier(getter.getName(), "field getter name");
                        requireEntityPath(getter.getEntityPath(), "field getter", getter.getName());
                        if (getter.isInstanceRequired() && (getter.getParameters() == null || getter.getParameters().isEmpty())) {
                            throw new CompilerException("Instance field getter " + getter.getName() + " missing handle parameter");
                        }
                        validateArgs(getter.getParameters(), "field getter parameters", getter.getName());
                        validateArgs(getter.getReturnValues(), "field getter return values", getter.getName());
                    }
                    if (field.getSetter() != null) {
                        MethodDefinition setter = field.getSetter();
                        requireJavaIdentifier(setter.getName(), "field setter name");
                        requireEntityPath(setter.getEntityPath(), "field setter", setter.getName());
                        if (setter.isInstanceRequired() && (setter.getParameters() == null || setter.getParameters().isEmpty())) {
                            throw new CompilerException("Instance field setter " + setter.getName() + " missing handle parameter");
                        }
                        validateArgs(setter.getParameters(), "field setter parameters", setter.getName());
                        validateArgs(setter.getReturnValues(), "field setter return values", setter.getName());
                    }
                }
            }

            for (GlobalDefinition globalDef : module.getGlobals()) {
                requireJavaIdentifier(globalDef.getName(), "global name");
                if (globalDef.getGetter() == null && globalDef.getSetter() == null) {
                    throw new CompilerException("Global " + globalDef.getName() + " has neither getter nor setter");
                }
                if (globalDef.getGetter() != null) {
                    FunctionDefinition getter = globalDef.getGetter();
                    requireJavaIdentifier(getter.getName(), "global getter name");
                    requireEntityPath(getter.getEntityPath(), "global getter", getter.getName());
                    validateArgs(getter.getParameters(), "global getter parameters", getter.getName());
                    validateArgs(getter.getReturnValues(), "global getter return values", getter.getName());
                }
                if (globalDef.getSetter() != null) {
                    FunctionDefinition setter = globalDef.getSetter();
                    requireJavaIdentifier(setter.getName(), "global setter name");
                    requireEntityPath(setter.getEntityPath(), "global setter", setter.getName());
                    validateArgs(setter.getParameters(), "global setter parameters", setter.getName());
                    validateArgs(setter.getReturnValues(), "global setter return values", setter.getName());
                }
            }
        }
    }

    private void validateArgs(List<ArgDefinition> args, String context, String ownerName) {
        if (args == null) {
            return;
        }
        for (ArgDefinition arg : args) {
            if (isNullOrEmpty(arg.getName())) {
                throw new CompilerException("Missing arg name in " + context + " for " + ownerName);
            }
            requireJavaIdentifier(arg.getName(), "arg name");
            if (isNullOrEmpty(arg.getType())) {
                throw new CompilerException("Missing arg type in " + context + " for " + ownerName + ": " + arg.getName());
            }
            if (arg.getDimensions() < 0) {
                throw new CompilerException("Negative dimensions for arg " + arg.getName() + " in " + context);
            }
        }
    }

    private void requireEntityPath(Map<String, String> entityPath, String kind, String name) {
        if (entityPath == null || entityPath.isEmpty()) {
            throw new CompilerException(kind + " " + name + " missing entity_path");
        }
    }

    private boolean getBooleanOption(Map<String, String> options, String key, boolean defaultValue) {
        String value = options.get(key);
        if (value == null || value.isEmpty()) {
            return defaultValue;
        }
        String normalized = value.toLowerCase(Locale.ROOT);
        if ("true".equals(normalized)) {
            return true;
        }
        if ("false".equals(normalized)) {
            return false;
        }
        throw new CompilerException("Invalid boolean value for " + key + ": " + value);
    }

    private String getStringOption(Map<String, String> options, String key) {
        String value = options.get(key);
        if (value == null) {
            return null;
        }
        return value.trim();
    }

    private void requireJavaIdentifier(String value, String context) {
        if (isNullOrEmpty(value)) {
            throw new CompilerException("Missing " + context);
        }
        if (!isValidJavaIdentifier(value)) {
            throw new CompilerException("Invalid Java identifier for " + context + ": " + value);
        }
    }

    private void requireJavaQualifiedName(String value, String context) {
        if (isNullOrEmpty(value)) {
            throw new CompilerException("Missing " + context);
        }
        String[] parts = value.split("\\.");
        for (String part : parts) {
            if (!isValidJavaIdentifier(part)) {
                throw new CompilerException("Invalid Java identifier for " + context + ": " + value);
            }
        }
    }

    private boolean isValidJavaIdentifier(String value) {
        if (value == null || value.isEmpty()) {
            return false;
        }
        if (JAVA_KEYWORDS.contains(value)) {
            return false;
        }
        if (!Character.isJavaIdentifierStart(value.charAt(0))) {
            return false;
        }
        for (int i = 1; i < value.length(); i++) {
            if (!Character.isJavaIdentifierPart(value.charAt(i))) {
                return false;
            }
        }
        return true;
    }

    private boolean isNullOrEmpty(String value) {
        return value == null || value.isEmpty();
    }

    private static final Set<String> JAVA_KEYWORDS = new HashSet<>();
    static {
        String[] keywords = new String[]{
            "abstract", "assert", "boolean", "break", "byte", "case", "catch", "char", "class",
            "const", "continue", "default", "do", "double", "else", "enum", "extends", "final",
            "finally", "float", "for", "goto", "if", "implements", "import", "instanceof", "int",
            "interface", "long", "native", "new", "package", "private", "protected", "public", "return",
            "short", "static", "strictfp", "super", "switch", "synchronized", "this", "throw", "throws",
            "transient", "try", "void", "volatile", "while", "true", "false", "null"
        };
        for (String kw : keywords) {
            JAVA_KEYWORDS.add(kw);
        }
    }
}
