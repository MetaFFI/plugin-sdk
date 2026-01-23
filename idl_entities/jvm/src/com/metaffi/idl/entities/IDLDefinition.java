package com.metaffi.idl.entities;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class IDLDefinition {
    private String idl_source;
    private String idl_extension;
    private String idl_filename_with_extension;
    private String idl_full_path;
    private String metaffi_guest_lib;
    private String target_language;
    private List<ModuleDefinition> modules;

    public IDLDefinition() {
        this.modules = new ArrayList<>();
    }

    public String getIdlSource() {
        return idl_source;
    }

    public void setIdlSource(String idlSource) {
        this.idl_source = idlSource;
    }

    public String getIdlExtension() {
        return idl_extension;
    }

    public void setIdlExtension(String idlExtension) {
        this.idl_extension = idlExtension;
    }

    public String getIdlFilenameWithExtension() {
        return idl_filename_with_extension;
    }

    public void setIdlFilenameWithExtension(String idlFilenameWithExtension) {
        this.idl_filename_with_extension = idlFilenameWithExtension;
    }

    public String getIdlFullPath() {
        return idl_full_path;
    }

    public void setIdlFullPath(String idlFullPath) {
        this.idl_full_path = idlFullPath;
    }

    public String getMetaffiGuestLib() {
        return metaffi_guest_lib;
    }

    public void setMetaffiGuestLib(String metaffiGuestLib) {
        this.metaffi_guest_lib = metaffiGuestLib;
    }

    public String getTargetLanguage() {
        return target_language;
    }

    public void setTargetLanguage(String targetLanguage) {
        this.target_language = targetLanguage;
    }

    public List<ModuleDefinition> getModules() {
        return modules;
    }

    public void setModules(List<ModuleDefinition> modules) {
        this.modules = modules;
    }

    public void finalizeConstruction() {
        if (modules == null) {
            return;
        }

        for (ModuleDefinition module : modules) {
            if (module.getExternalResources() == null) {
                continue;
            }

            List<String> expanded = new ArrayList<>();
            for (String resource : module.getExternalResources()) {
                expanded.add(expandEnv(resource));
            }
            module.setExternalResources(expanded);
        }
    }

    private String expandEnv(String value) {
        if (value == null) {
            return null;
        }

        String result = replaceEnvPattern(value, Pattern.compile("\\$\\{([^}]+)\\}"), 1);
        return replaceEnvPattern(result, Pattern.compile("\\$([A-Za-z_][A-Za-z0-9_]*)"), 1);
    }

    private String replaceEnvPattern(String input, Pattern pattern, int groupIndex) {
        Matcher matcher = pattern.matcher(input);
        StringBuffer buffer = new StringBuffer();

        while (matcher.find()) {
            String envName = matcher.group(groupIndex);
            String envValue = System.getenv(envName);
            if (envValue == null) {
                envValue = "";
            }
            matcher.appendReplacement(buffer, Matcher.quoteReplacement(envValue));
        }
        matcher.appendTail(buffer);
        return buffer.toString();
    }
}
