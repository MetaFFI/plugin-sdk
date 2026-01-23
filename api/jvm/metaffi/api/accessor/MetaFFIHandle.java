package metaffi.api.accessor;

public class MetaFFIHandle
{
	public long handle;
	public long runtime_id;
	public long releaser;

	public MetaFFIHandle(long val, long runtime_id, long releaser)
	{
		this.handle = val;
		this.runtime_id = runtime_id;
		this.releaser = releaser;
	}

	public long Handle()
	{
		return this.handle;
	}

	public long RuntimeID()
	{
		return this.runtime_id;
	}

	public long Releaser()
	{
		return this.releaser;
	}
}

