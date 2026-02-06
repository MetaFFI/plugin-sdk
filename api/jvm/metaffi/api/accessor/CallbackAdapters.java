package metaffi.api.accessor;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

public final class CallbackAdapters
{
	private CallbackAdapters()
	{
	}

	public static Object asInterface(Caller caller, String interfaceName)
	{
		if(caller == null)
		{
			throw new IllegalArgumentException("caller is null");
		}
		if(interfaceName == null || interfaceName.isEmpty())
		{
			throw new IllegalArgumentException("interfaceName is empty");
		}

		try
		{
			ClassLoader loader = Thread.currentThread().getContextClassLoader();
			if(loader == null)
			{
				loader = CallbackAdapters.class.getClassLoader();
			}

			Class<?> iface;
			try
			{
				iface = Class.forName(interfaceName, true, loader);
			}
			catch(ClassNotFoundException e)
			{
				ClassLoader fallback = CallbackAdapters.class.getClassLoader();
				if(fallback != loader)
				{
					iface = Class.forName(interfaceName, true, fallback);
				}
				else
				{
					throw e;
				}
			}
			if(!iface.isInterface())
			{
				throw new IllegalArgumentException("Not an interface: " + interfaceName);
			}

			InvocationHandler handler = (Object proxy, Method method, Object[] args) ->
			{
				if(method.getDeclaringClass() == Object.class)
				{
					String name = method.getName();
					if("toString".equals(name))
					{
						return "MetaFFIProxy(" + interfaceName + ")";
					}
					if("hashCode".equals(name))
					{
						return System.identityHashCode(proxy);
					}
					if("equals".equals(name))
					{
						return proxy == (args != null && args.length > 0 ? args[0] : null);
					}
				}

				Object[] safeArgs = (args == null) ? new Object[0] : args;
				Object[] results = caller.call(safeArgs);

				if(method.getReturnType() == void.class)
				{
					return null;
				}

				if(results == null || results.length == 0)
				{
					return null;
				}
				return results[0];
			};

			return Proxy.newProxyInstance(loader, new Class<?>[]{iface}, handler);
		}
		catch(ClassNotFoundException e)
		{
			throw new IllegalArgumentException("Interface not found: " + interfaceName, e);
		}
		catch(Throwable t)
		{
			throw new IllegalArgumentException("Failed to create proxy for " + interfaceName + ": " + t, t);
		}
	}
}
