package com.metaffi.idl;

import com.metaffi.idl.model.ModuleInfo;
import java.io.IOException;

/**
 * Interface for all extractors (ClassFile, Jar, Directory).
 * Each extractor implementation handles a specific source type and returns a ModuleInfo.
 */
public interface Extractor {
    /**
     * Extract interface definitions from the source.
     *
     * @return ModuleInfo containing all extracted classes
     * @throws IOException if source cannot be read
     */
    ModuleInfo extract() throws IOException;
}
