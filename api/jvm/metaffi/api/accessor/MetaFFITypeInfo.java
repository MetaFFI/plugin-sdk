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
        this.dimensions = dims;

		switch(jniCharType.toUpperCase())
		{
			case "Z":
				this.type = MetaFFITypes.MetaFFIBool;
				break;
            case "B":
                this.type = MetaFFITypes.MetaFFIInt8;
                break;
            case "C":
                this.type = MetaFFITypes.MetaFFIUInt16;
                break;
            case "S":
                this.type = MetaFFITypes.MetaFFIInt16;
                break;
            case "I":
                this.type = MetaFFITypes.MetaFFIInt32;
                break;
            case "J":
                this.type = MetaFFITypes.MetaFFIInt64;
                break;
            case "F":
                this.type = MetaFFITypes.MetaFFIFloat32;
                break;
            case "D":
                this.type = MetaFFITypes.MetaFFIFloat64;
                break;
            case "[Z":
                this.type = MetaFFITypes.MetaFFIBoolArray;
                break;
            case "[B":
                this.type = MetaFFITypes.MetaFFIInt8Array;
                break;
            case "[C":
                this.type = MetaFFITypes.MetaFFIUInt16Array;
                break;
            case "[S":
                this.type = MetaFFITypes.MetaFFIInt16Array;
                break;
            case "[I":
                this.type = MetaFFITypes.MetaFFIInt32Array;
                break;
            case "[J":
                this.type = MetaFFITypes.MetaFFIInt64Array;
                break;
            case "[F":
                this.type = MetaFFITypes.MetaFFIFloat32Array;
                break;
            case "[D":
                this.type = MetaFFITypes.MetaFFIFloat64Array;
                break;
            default:
                if(jniCharType.contains("java/lang/Object"))
                    this.type = MetaFFITypes.MetaFFIAny;
                else
                    this.type = MetaFFITypes.MetaFFIHandle;
		}

        this.value = this.type.value;
    }
}

