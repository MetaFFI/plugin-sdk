package metaffi.api.accessor;

public class MetaFFITypeInfo
{
	public enum MetaFFITypes
	{
		MetaFFIFloat64(1),
		MetaFFIFloat32(2),
		MetaFFIInt8(4),
		MetaFFIInt16(8),
		MetaFFIInt32(16),
		MetaFFIInt64(32),
		MetaFFIUInt8(64),
		MetaFFIUInt16(128),
		MetaFFIUInt32(256),
		MetaFFIUInt64(512),
		MetaFFIBool(1024),
		MetaFFIChar8(524288),
		MetaFFIChar16(1048576),
		MetaFFIChar32(2097152),
		MetaFFIString8(4096),
		MetaFFIString16(8192),
		MetaFFIString32(16384),
		MetaFFIHandle(32768),
		MetaFFIArray(65536),
		MetaFFISize(262144),
		MetaFFIAny(4194304),
		MetaFFINull(8388608),
		MetaFFICallable(16777216),
		MetaFFIFloat64Array(MetaFFIFloat64.value | MetaFFIArray.value),
		MetaFFIFloat32Array(MetaFFIFloat32.value | MetaFFIArray.value),
		MetaFFIInt8Array(MetaFFIInt8.value | MetaFFIArray.value),
		MetaFFIInt16Array(MetaFFIInt16.value | MetaFFIArray.value),
		MetaFFIInt32Array(MetaFFIInt32.value | MetaFFIArray.value),
		MetaFFIInt64Array(MetaFFIInt64.value | MetaFFIArray.value),
		MetaFFIUInt8Array(MetaFFIUInt8.value | MetaFFIArray.value),
		MetaFFIUInt16Array(MetaFFIUInt16.value | MetaFFIArray.value),
		MetaFFIUInt32Array(MetaFFIUInt32.value | MetaFFIArray.value),
		MetaFFIUInt64Array(MetaFFIUInt64.value | MetaFFIArray.value),
		MetaFFIBoolArray(MetaFFIBool.value | MetaFFIArray.value),
		MetaFFIChar8Array(MetaFFIChar8.value | MetaFFIArray.value),
		MetaFFIString8Array(MetaFFIString8.value | MetaFFIArray.value),
		MetaFFIString16Array(MetaFFIString16.value | MetaFFIArray.value),
		MetaFFIString32Array(MetaFFIString32.value | MetaFFIArray.value),
		MetaFFIAnyArray(MetaFFIAny.value | MetaFFIArray.value),
		MetaFFIHandleArray(MetaFFIHandle.value | MetaFFIArray.value),
		MetaFFISizeArray(MetaFFISize.value | MetaFFIArray.value);

		public final long value;

		MetaFFITypes(long value){
			this.value = value;
		}
	}

	public final MetaFFITypes type;
	public final String alias;
	public final long value;
	public final int dimensions;

	public MetaFFITypeInfo(MetaFFITypes metaffiType)
	{
		this.type = metaffiType;
		this.value = metaffiType.value;
		this.alias = null;
		this.dimensions = 0;
	}

	public MetaFFITypeInfo(MetaFFITypes metaffiType, int dims)
    {
        this.type = metaffiType;
        this.value = metaffiType.value;
        this.alias = null;
        this.dimensions = dims;
    }

	public MetaFFITypeInfo(MetaFFITypes metaffiType, String alias)
	{
		this.type = metaffiType;
		this.value = metaffiType.value;
		this.alias = alias;
		this.dimensions = 0;
	}

	public MetaFFITypeInfo(MetaFFITypes metaffiType, String alias, int dims)
    {
        this.type = metaffiType;
        this.value = metaffiType.value;
        this.alias = alias;
        this.dimensions = dims;
    }

	public MetaFFITypeInfo(String jniCharType)
    {
        this(jniCharType, null, 0);
    }

    public MetaFFITypeInfo(String jniCharType, int dims)
    {
        this(jniCharType, null, dims);
    }

	public MetaFFITypeInfo(String jniCharType, String alias, int dims)
    {
        this.alias = alias;
        int computedDims = dims;
        String sig = jniCharType;
        while (sig.startsWith("[")) {
            computedDims++;
            sig = sig.substring(1);
        }
        this.dimensions = computedDims;

		MetaFFITypes baseType;
		switch(sig)
		{
			case "Z":
				baseType = MetaFFITypes.MetaFFIBool;
				break;
            case "B":
                baseType = MetaFFITypes.MetaFFIInt8;
                break;
            case "C":
                baseType = MetaFFITypes.MetaFFIUInt16;
                break;
            case "S":
                baseType = MetaFFITypes.MetaFFIInt16;
                break;
            case "I":
                baseType = MetaFFITypes.MetaFFIInt32;
                break;
            case "J":
                baseType = MetaFFITypes.MetaFFIInt64;
                break;
            case "F":
                baseType = MetaFFITypes.MetaFFIFloat32;
                break;
            case "D":
                baseType = MetaFFITypes.MetaFFIFloat64;
                break;
            case "Ljava/lang/String;":
                baseType = MetaFFITypes.MetaFFIString8;
                break;
            case "Lmetaffi/api/accessor/Caller;":
                baseType = MetaFFITypes.MetaFFICallable;
                break;
            default:
                if(sig.contains("java/lang/Object"))
                    baseType = MetaFFITypes.MetaFFIAny;
                else
                    baseType = MetaFFITypes.MetaFFIHandle;
		}

        if (computedDims > 0) {
            baseType = toArrayType(baseType);
        }

        this.type = baseType;
        this.value = this.type.value;
    }

    private static MetaFFITypes toArrayType(MetaFFITypes baseType)
    {
        switch(baseType)
        {
            case MetaFFIBool: return MetaFFITypes.MetaFFIBoolArray;
            case MetaFFIInt8: return MetaFFITypes.MetaFFIInt8Array;
            case MetaFFIInt16: return MetaFFITypes.MetaFFIInt16Array;
            case MetaFFIInt32: return MetaFFITypes.MetaFFIInt32Array;
            case MetaFFIInt64: return MetaFFITypes.MetaFFIInt64Array;
            case MetaFFIUInt8: return MetaFFITypes.MetaFFIUInt8Array;
            case MetaFFIUInt16: return MetaFFITypes.MetaFFIUInt16Array;
            case MetaFFIUInt32: return MetaFFITypes.MetaFFIUInt32Array;
            case MetaFFIUInt64: return MetaFFITypes.MetaFFIUInt64Array;
            case MetaFFIFloat32: return MetaFFITypes.MetaFFIFloat32Array;
            case MetaFFIFloat64: return MetaFFITypes.MetaFFIFloat64Array;
            case MetaFFIString8: return MetaFFITypes.MetaFFIString8Array;
            case MetaFFIAny: return MetaFFITypes.MetaFFIAnyArray;
            case MetaFFIHandle: return MetaFFITypes.MetaFFIHandleArray;
            case MetaFFISize: return MetaFFITypes.MetaFFISizeArray;
            default: return MetaFFITypes.MetaFFIHandleArray;
        }
    }
}

