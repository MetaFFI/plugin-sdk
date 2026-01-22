package guest;

public class AutoCloseableResource implements AutoCloseable {
	private boolean closed = false;

	@Override
	public void close() {
		closed = true;
	}

	public boolean isClosed() {
		return closed;
	}
}
