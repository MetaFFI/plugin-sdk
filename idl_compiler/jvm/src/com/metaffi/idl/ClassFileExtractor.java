package com.metaffi.idl;

import com.metaffi.idl.model.ClassInfo;
import com.metaffi.idl.model.FieldInfo;
import com.metaffi.idl.model.MethodInfo;
import com.metaffi.idl.model.ModuleInfo;
import com.metaffi.idl.model.ParameterInfo;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.FieldVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

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
            
            // Get Java class name for return type
            String javaReturnType = getJavaTypeName(returnType);
            method.setReturnType(javaReturnType);
            method.setMetaffiReturnType(returnInfo.metaffiType);
            method.setReturnDimensions(returnInfo.dimensions);

            // Extract parameters
            Type[] argumentTypes = methodType.getArgumentTypes();
            for (int i = 0; i < argumentTypes.length; i++) {
                Type argType = argumentTypes[i];
                TypeMapper.TypeInfo argInfo = TypeMapper.mapType(argType.getDescriptor());

                ParameterInfo param = new ParameterInfo();
                param.setName("p" + i);  // Default name, may be overridden by LocalVariableTable
                param.setJavaType(getJavaTypeName(argType));
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

            return null;
        }
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
