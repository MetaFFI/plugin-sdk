#include <memory>

namespace metaffi{ namespace utils
{
template<typename T>
class singleton
{
private:
	static std::unique_ptr<T> _instance;

	static void init()
	{
		_instance = std::make_unique<T>();
	}
	
	singleton(){}
	
public:
	static T& instance()
	{
		static const bool flag = []() -> bool
		{
			init();
			return true;
		}();
		
		return *_instance;
	}
	
	static void free()
	{
		_instance->free();
		_instance = nullptr;
	}
};
template <typename T>
std::unique_ptr<T> singleton<T>::_instance;
}}