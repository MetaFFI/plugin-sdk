package com.metaffi.idl;

import com.metaffi.idl.model.ClassInfo;
import com.metaffi.idl.model.FieldInfo;
import com.metaffi.idl.model.MethodInfo;
import com.metaffi.idl.model.ModuleInfo;
import com.metaffi.idl.model.ParameterInfo;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.FieldVisitor;
import org.objectweb.asm.Label;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;

/**
 * Extracts interface definitions from a single .class file using ASM library.
 * Handles public classes, methods, constructors, and fields.
 * Includes inner classes (with $ in names).
 */
public class ClassFileExtractor implements Extractor {
    private final String classFilePath;
    private final String moduleName;

    /**
     * Constructor.
     *
     * @param classFilePath Path to the .class file
     * @param moduleName Module name (used for ModuleInfo)
     */
    public ClassFileExtractor(String classFilePath, String moduleName) {
        this.classFilePath = classFilePath;
        this.moduleName = moduleName;
    }

    @Override
    public ModuleInfo extract() throws IOException {
        try (FileInputStream fis = new FileInputStream(classFilePath)) {
            return extractFromStream(fis);
        }
    }

    /**
     * Extract from an InputStream (useful for JAR entries).
     *
     * @param inputStream InputStream containing class bytecode
     * @return ModuleInfo containing extracted class
     * @throws IOException if stream cannot be read
     */
    public ModuleInfo extractFromStream(InputStream inputStream) throws IOException {
        ClassReader reader = new ClassReader(inputStream);
        ClassInfo classInfo = new ClassInfo();
        MetaFFIClassVisitor visitor = new MetaFFIClassVisitor(classInfo);

        // Visit class with SKIP_DEBUG=false to potentially get parameter names
        reader.accept(visitor, 0);

        // Only create module if class was public and extracted
        if (classInfo.getClassName() == null) {
            // Non-public class was skipped
            ModuleInfo emptyModule = new ModuleInfo(moduleName);
            emptyModule.setSourcePath(classFilePath);
            return emptyModule;
        }

        ModuleInfo module = new ModuleInfo(moduleName);
        module.setSourcePath(classFilePath);
        module.getClasses().add(classInfo);

        return module;
    }

    /**
     * ASM ClassVisitor that extracts public interface definitions.
     */
    private static class MetaFFIClassVisitor extends ClassVisitor {
        private final ClassInfo classInfo;
        private boolean isPublicClass = false;

        public MetaFFIClassVisitor(ClassInfo classInfo) {
            super(Opcodes.ASM9);
            this.classInfo = classInfo;
        }

        @Override
        public void visit(int version, int access, String name,
                         String signature, String superName, String[] interfaces) {
            // Only process public classes
            if ((access & Opcodes.ACC_PUBLIC) == 0) {
                return; // Skip non-public classes
            }

            isPublicClass = true;

            // Convert internal name (with /) to fully qualified name (with .)
            String className = name.replace('/', '.');
            classInfo.setClassName(className);
            classInfo.setIsInterface((access & Opcodes.ACC_INTERFACE) != 0);
            classInfo.setIsAbstract((access & Opcodes.ACC_ABSTRACT) != 0);

            // Extract package and simple name
            int lastDot = className.lastIndexOf('.');
            if (lastDot > 0) {
                classInfo.setPackageName(className.substring(0, lastDot));
                classInfo.setSimpleName(className.substring(lastDot + 1));
            } else {
                classInfo.setPackageName("");
                classInfo.setSimpleName(className);
            }
        }

        @Override
        public FieldVisitor visitField(int access, String name, String descriptor,
                                       String signature, Object value) {
            // Only public fields
            if ((access & Opcodes.ACC_PUBLIC) == 0) {
                return null;
            }

            // Skip synthetic fields
            if ((access & Opcodes.ACC_SYNTHETIC) != 0) {
                return null;
            }

            FieldInfo field = new FieldInfo();
            field.setName(name);
            
            // Get readable Java type name
            Type fieldType = Type.getType(descriptor);
            field.setJavaType(getJavaTypeName(fieldType));
            field.setIsStatic((access & Opcodes.ACC_STATIC) != 0);

            // Map type using TypeMapper
            TypeMapper.TypeInfo typeInfo = TypeMapper.mapType(descriptor);
            field.setMetaffiType(typeInfo.metaffiType);
            field.setDimensions(typeInfo.dimensions);

            classInfo.getFields().add(field);
            return null;
        }

        @Override
        public MethodVisitor visitMethod(int access, String name, String descriptor,
                                        String signature, String[] exceptions) {
            // Only public methods
            if ((access & Opcodes.ACC_PUBLIC) == 0) {
                return null;
            }

            // Skip synthetic methods
            if ((access & Opcodes.ACC_SYNTHETIC) != 0) {
                return null;
            }

            // Skip static initializer
            if ("<clinit>".equals(name)) {
                return null;
            }

            MethodInfo method = new MethodInfo();
            method.setName(name);
            method.setIsStatic((access & Opcodes.ACC_STATIC) != 0);
            method.setIsConstructor("<init>".equals(name));

            // Parse method descriptor
            Type methodType = Type.getMethodType(descriptor);

            // Extract return type
            Type returnType = methodType.getReturnType();
            TypeMapper.TypeInfo returnInfo = TypeMapper.mapType(returnType.getDescriptor());

            // Get Java class name for return type (use signature for generics if available)
            String javaReturnType;
            if (signature != null) {
                javaReturnType = parseGenericReturnType(signature, returnType);
            } else {
                javaReturnType = getJavaTypeName(returnType);
            }
            method.setReturnType(javaReturnType);
            method.setMetaffiReturnType(returnInfo.metaffiType);
            method.setReturnDimensions(returnInfo.dimensions);

            // Extract parameters
            Type[] argumentTypes = methodType.getArgumentTypes();
            for (int i = 0; i < argumentTypes.length; i++) {
                Type argType = argumentTypes[i];
                TypeMapper.TypeInfo argInfo = TypeMapper.mapType(argType.getDescriptor());

                ParameterInfo param = new ParameterInfo();
                param.setName("p" + i);  // Default name, will be overridden if bytecode has names

                // Use signature for generics if available
                if (signature != null) {
                    param.setJavaType(parseGenericParamType(signature, i, argType));
                } else {
                    param.setJavaType(getJavaTypeName(argType));
                }
                param.setMetaffiType(argInfo.metaffiType);
                param.setDimensions(argInfo.dimensions);

                method.getParameters().add(param);
            }

            // Add to appropriate list
            if (method.isConstructor()) {
                classInfo.getConstructors().add(method);
            } else {
                classInfo.getMethods().add(method);
            }

            // Return a MethodVisitor to capture parameter names from bytecode attributes
            return new ParameterNameVisitor(method);
        }
    }

    /**
     * MethodVisitor that extracts parameter names from bytecode attributes.
     * Captures names from MethodParameters attribute (-parameters flag) or
     * LocalVariableTable attribute (debug info, -g flag).
     */
    private static class ParameterNameVisitor extends MethodVisitor {
        private final MethodInfo method;
        private int paramIndex = 0;
        private boolean hasMethodParams = false;

        public ParameterNameVisitor(MethodInfo method) {
            super(Opcodes.ASM9);
            this.method = method;
        }

        @Override
        public void visitParameter(String name, int access) {
            // Called when class is compiled with -parameters flag
            // This gives us the actual parameter names
            if (name != null && paramIndex < method.getParameters().size()) {
                method.getParameters().get(paramIndex).setName(name);
                hasMethodParams = true;
            }
            paramIndex++;
        }

        @Override
        public void visitLocalVariable(String name, String descriptor, String signature,
                                       Label start, Label end, int index) {
            // Called when class has debug info (compiled with -g flag)
            // LocalVariableTable includes "this" at index 0 for instance methods
            // Only use this if we didn't get names from MethodParameters
            if (hasMethodParams) {
                return;
            }

            List<ParameterInfo> params = method.getParameters();

            // For instance methods, index 0 is "this", so params start at index 1
            // For static methods, params start at index 0
            int paramOffset = method.isStatic() ? 0 : 1;
            int paramIdx = index - paramOffset;

            // Only update if this is a parameter (not a local variable)
            if (paramIdx >= 0 && paramIdx < params.size() && !"this".equals(name)) {
                params.get(paramIdx).setName(name);
            }
        }
    }

    /**
     * Parse generic return type from method signature.
     * Attempts to extract readable generic type like "List<String>" from signature.
     */
    private static String parseGenericReturnType(String signature, Type returnType) {
        // Signature format: (<params>)<return>
        // Example: (Ljava/util/List<Ljava/lang/String;>;)V
        int returnStart = signature.lastIndexOf(')');
        if (returnStart < 0 || returnStart >= signature.length() - 1) {
            return getJavaTypeName(returnType);
        }

        String returnSig = signature.substring(returnStart + 1);
        return parseGenericType(returnSig, returnType);
    }

    /**
     * Parse generic parameter type from method signature.
     */
    private static String parseGenericParamType(String signature, int paramIndex, Type paramType) {
        // For now, fall back to basic type name
        // Full generic parsing would require proper signature parsing
        // This is a simplified implementation
        return getJavaTypeName(paramType);
    }

    /**
     * Parse a generic type from signature string.
     */
    private static String parseGenericType(String sig, Type fallbackType) {
        // Simple parsing for common cases like List<String>
        if (!sig.contains("<")) {
            return getJavaTypeName(fallbackType);
        }

        try {
            StringBuilder result = new StringBuilder();
            int i = 0;
            while (i < sig.length()) {
                char c = sig.charAt(i);
                if (c == 'L') {
                    // Object type
                    int semicolon = sig.indexOf(';', i);
                    int angleBracket = sig.indexOf('<', i);

                    if (angleBracket > 0 && angleBracket < semicolon) {
                        // Has generic parameters
                        String className = sig.substring(i + 1, angleBracket).replace('/', '.');
                        result.append(simplifyClassName(className));
                        result.append('<');

                        // Parse generic args (simplified - handles one level)
                        int depth = 1;
                        int start = angleBracket + 1;
                        for (int j = angleBracket + 1; j < sig.length() && depth > 0; j++) {
                            char gc = sig.charAt(j);
                            if (gc == '<') depth++;
                            else if (gc == '>') {
                                depth--;
                                if (depth == 0) {
                                    String genericArg = sig.substring(start, j);
                                    result.append(parseGenericArg(genericArg));
                                }
                            }
                        }
                        result.append('>');
                        return result.toString();
                    } else {
                        // No generics
                        String className = sig.substring(i + 1, semicolon).replace('/', '.');
                        return simplifyClassName(className);
                    }
                }
                i++;
            }
        } catch (Exception e) {
            // Fall back on parse error
        }

        return getJavaTypeName(fallbackType);
    }

    /**
     * Parse a single generic argument.
     */
    private static String parseGenericArg(String arg) {
        if (arg.startsWith("L") && arg.endsWith(";")) {
            String className = arg.substring(1, arg.length() - 1).replace('/', '.');
            return simplifyClassName(className);
        }
        // Handle primitive types and wildcards
        switch (arg) {
            case "I": return "Integer";
            case "J": return "Long";
            case "D": return "Double";
            case "F": return "Float";
            case "Z": return "Boolean";
            case "B": return "Byte";
            case "S": return "Short";
            case "C": return "Character";
            case "*": return "?";
            default: return arg;
        }
    }

    /**
     * Simplify fully qualified class name to simple name for common types.
     */
    private static String simplifyClassName(String className) {
        // Keep common java.lang and java.util types simple
        if (className.startsWith("java.lang.")) {
            return className.substring("java.lang.".length());
        }
        if (className.startsWith("java.util.")) {
            return className.substring("java.util.".length());
        }
        return className;
    }

    /**
     * Convert ASM Type to Java type name string.
     */
    private static String getJavaTypeName(Type type) {
            switch (type.getSort()) {
                case Type.VOID:
                    return "void";
                case Type.BOOLEAN:
                    return "boolean";
                case Type.BYTE:
                    return "byte";
                case Type.SHORT:
                    return "short";
                case Type.INT:
                    return "int";
                case Type.LONG:
                    return "long";
                case Type.FLOAT:
                    return "float";
                case Type.DOUBLE:
                    return "double";
                case Type.CHAR:
                    return "char";
                case Type.ARRAY:
                    // For arrays, get element type and add brackets
                    Type elementType = type.getElementType();
                    String elementName = getJavaTypeName(elementType);
                    StringBuilder sb = new StringBuilder(elementName);
                    for (int i = 0; i < type.getDimensions(); i++) {
                        sb.append("[]");
                    }
                    return sb.toString();
                case Type.OBJECT:
                    return type.getClassName();
                default:
                    return type.toString();
            }
    }
}
