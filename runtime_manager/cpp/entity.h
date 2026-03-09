#pragma once

#include <string>
#include <mutex>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <new>

/**
 * Abstract base class for C/C++ entities.
 *
 * An entity represents an exported symbol (function or global variable) from a
 * C/C++ shared library. The entity path carries the already-mangled symbol name;
 * this runtime_manager never mangles or demangles symbols itself.
 *
 * Phase 1:
 *   CppFreeFunction  : get_function_pointer() returns a callable function pointer.
 *   CppGlobalGetter  : get_function_pointer() returns void* into the variable's memory.
 *   CppGlobalSetter  : get_function_pointer() returns void* into the variable's memory.
 *
 * Phase 2:
 *   CppInstanceMethod : get_function_pointer() returns fn ptr; caller passes this* as 1st arg.
 *   CppConstructor    : allocate() + get_function_pointer() for ctor fn ptr.
 *   CppDestructor     : destroy(void*) calls dtor then frees memory.
 *   CppFieldGetter    : get(void*) returns pointer to field via byte offset.
 *   CppFieldSetter    : set(void*, const void*, size_t) memcpys into field.
 *
 * Entities are always managed via shared_ptr and are not copyable or movable.
 */
class Entity
{
public:
	virtual ~Entity() = default;

	/**
	 * Get the raw symbol address.
	 *  - For CppFreeFunction  : a function pointer (caller casts and invokes).
	 *  - For CppGlobalGetter  : pointer-to-variable memory (caller dereferences).
	 *  - For CppGlobalSetter  : pointer-to-variable memory (caller writes through it).
	 * @return Symbol address as void*
	 */
	virtual void* get_function_pointer() const = 0;

	/**
	 * Get the entity name (the symbol name passed to dlsym / GetProcAddress).
	 * @return Reference to the name string
	 */
	virtual const std::string& get_name() const = 0;

	// Entities are reference-counted via shared_ptr — no copy or move.
	Entity(const Entity&)            = delete;
	Entity& operator=(const Entity&) = delete;
	Entity(Entity&&)                 = delete;
	Entity& operator=(Entity&&)      = delete;

protected:
	Entity() = default;
};


/**
 * A C/C++ free function or static method.
 *
 * get_function_pointer() returns the function pointer. Callers cast to the
 * appropriate signature (e.g. reinterpret_cast<int(*)(int,int)>(...)) and invoke.
 */
class CppFreeFunction : public Entity
{
public:
	/**
	 * @param func_ptr  Non-null function pointer from dlsym/GetProcAddress.
	 * @param name      Symbol name (used in diagnostics and get_name()).
	 * @throws std::runtime_error if func_ptr is null or name is empty.
	 */
	CppFreeFunction(void* func_ptr, const std::string& name);
	~CppFreeFunction() override = default;

	void* get_function_pointer() const override;
	const std::string& get_name()      const override;

private:
	void*       m_funcPtr = nullptr;
	std::string m_name;
	mutable std::mutex m_mutex;
};


/**
 * A C/C++ global variable getter.
 *
 * Both get_function_pointer() and get() return a void* pointing directly into
 * the module's memory for the global variable (zero-copy). The caller reads the
 * value at that address with an appropriate cast.
 */
class CppGlobalGetter : public Entity
{
public:
	/**
	 * @param var_ptr  Non-null pointer to the global variable (from dlsym).
	 * @param name     Symbol name.
	 * @throws std::runtime_error if var_ptr is null or name is empty.
	 */
	CppGlobalGetter(void* var_ptr, const std::string& name);
	~CppGlobalGetter() override = default;

	/** Returns pointer into the global variable's memory. */
	void* get_function_pointer() const override;
	const std::string& get_name()      const override;

	/**
	 * Explicit getter accessor — same result as get_function_pointer() but
	 * communicates intent at the call site.
	 * @return void* to the variable's memory inside the loaded module.
	 */
	void* get() const;

private:
	void*       m_varPtr = nullptr;
	std::string m_name;
	mutable std::mutex m_mutex;
};


/**
 * A C/C++ global variable setter.
 *
 * get_function_pointer() returns void* into the variable's memory. The set()
 * method memcopies caller-supplied bytes into that memory.
 */
class CppGlobalSetter : public Entity
{
public:
	/**
	 * @param var_ptr  Non-null pointer to the global variable (from dlsym).
	 * @param name     Symbol name.
	 * @throws std::runtime_error if var_ptr is null or name is empty.
	 */
	CppGlobalSetter(void* var_ptr, const std::string& name);
	~CppGlobalSetter() override = default;

	/** Returns pointer into the global variable's memory. */
	void* get_function_pointer() const override;
	const std::string& get_name()      const override;

	/**
	 * Write size bytes from value into the global variable's memory.
	 * @param value  Non-null pointer to source data.
	 * @param size   Number of bytes to copy; must be > 0.
	 * @throws std::runtime_error if value is null or size is 0.
	 */
	void set(const void* value, std::size_t size) const;

private:
	void*       m_varPtr = nullptr;
	std::string m_name;
	mutable std::mutex m_mutex;
};


// ============================================================================
// Phase 2 entity types
// ============================================================================

/**
 * A C++ instance method.
 *
 * get_function_pointer() returns the raw function pointer. The caller is
 * responsible for passing the object instance (this*) as the first argument
 * when invoking the function.
 */
class CppInstanceMethod : public Entity
{
public:
	/**
	 * @param func_ptr  Non-null function pointer from dlsym/GetProcAddress.
	 * @param name      Symbol name (mangled).
	 * @throws std::runtime_error if func_ptr is null or name is empty.
	 */
	CppInstanceMethod(void* func_ptr, const std::string& name);
	~CppInstanceMethod() override = default;

	void* get_function_pointer() const override;
	const std::string& get_name()      const override;

private:
	void*       m_funcPtr = nullptr;
	std::string m_name;
	mutable std::mutex m_mutex;
};


/**
 * A C++ constructor.
 *
 * allocate() allocates class_size bytes (must be called before the ctor fn ptr
 * is invoked). get_function_pointer() returns the constructor function pointer;
 * the caller passes the allocated pointer as the first argument (this*).
 */
class CppConstructor : public Entity
{
public:
	/**
	 * @param func_ptr    Non-null constructor function pointer.
	 * @param name        Symbol name (mangled constructor).
	 * @param class_size  sizeof(class) — number of bytes to allocate.
	 * @throws std::runtime_error if func_ptr is null or name is empty.
	 */
	CppConstructor(void* func_ptr, const std::string& name, std::size_t class_size);
	~CppConstructor() override = default;

	/**
	 * Allocate class_size bytes. Returns the raw memory pointer.
	 * Caller must pass this pointer as the first argument to get_function_pointer().
	 * @throws std::bad_alloc if allocation fails.
	 */
	void* allocate() const;

	void* get_function_pointer() const override;
	const std::string& get_name()      const override;

private:
	void*       m_funcPtr   = nullptr;
	std::string m_name;
	std::size_t m_classSize = 0;
	mutable std::mutex m_mutex;
};


/**
 * A C++ destructor.
 *
 * destroy(void* instance) calls the destructor function with the instance pointer,
 * then frees the allocated memory via std::free().
 */
class CppDestructor : public Entity
{
public:
	/**
	 * @param func_ptr  Non-null destructor function pointer.
	 * @param name      Symbol name (mangled destructor).
	 * @throws std::runtime_error if func_ptr is null or name is empty.
	 */
	CppDestructor(void* func_ptr, const std::string& name);
	~CppDestructor() override = default;

	/**
	 * Call the destructor on instance and free the memory.
	 * @param instance  Non-null pointer to the object to destroy.
	 * @throws std::runtime_error if instance is null.
	 */
	void destroy(void* instance) const;

	void* get_function_pointer() const override;
	const std::string& get_name()      const override;

private:
	void*       m_funcPtr = nullptr;
	std::string m_name;
	mutable std::mutex m_mutex;
};


/**
 * A C++ field getter using byte offset.
 *
 * No symbol lookup is performed — the entity computes field address as
 * (char*)instance + field_offset. No function pointer is involved.
 */
class CppFieldGetter : public Entity
{
public:
	/**
	 * @param field_name    Logical field name (for diagnostics).
	 * @param field_offset  Byte offset of the field within the class.
	 * @throws std::runtime_error if field_name is empty.
	 */
	CppFieldGetter(const std::string& field_name, std::size_t field_offset);
	~CppFieldGetter() override = default;

	/**
	 * Returns a pointer to the field inside instance.
	 * @param instance  Non-null pointer to the object.
	 * @return void* pointing to instance + field_offset.
	 * @throws std::runtime_error if instance is null.
	 */
	void* get(void* instance) const;

	/** Returns nullptr — field access uses offset arithmetic, not a function pointer. */
	void* get_function_pointer() const override;
	const std::string& get_name()      const override;

private:
	std::string m_name;
	std::size_t m_fieldOffset = 0;
	mutable std::mutex m_mutex;
};


/**
 * A C++ field setter using byte offset and memcpy.
 *
 * No symbol lookup is performed. set() memcopies the supplied value into
 * the field at (char*)instance + field_offset.
 */
class CppFieldSetter : public Entity
{
public:
	/**
	 * @param field_name    Logical field name (for diagnostics).
	 * @param field_offset  Byte offset of the field within the class.
	 * @param field_size    Size of the field in bytes (used for validation).
	 * @throws std::runtime_error if field_name is empty.
	 */
	CppFieldSetter(const std::string& field_name, std::size_t field_offset, std::size_t field_size);
	~CppFieldSetter() override = default;

	/**
	 * Copy sz bytes from value into the field at instance + field_offset.
	 * @param instance  Non-null pointer to the object.
	 * @param value     Non-null pointer to the source data.
	 * @param sz        Number of bytes to copy; must be > 0.
	 * @throws std::runtime_error if any argument is invalid.
	 */
	void set(void* instance, const void* value, std::size_t sz) const;

	/** Returns nullptr — field access uses offset arithmetic, not a function pointer. */
	void* get_function_pointer() const override;
	const std::string& get_name()      const override;

private:
	std::string m_name;
	std::size_t m_fieldOffset = 0;
	std::size_t m_fieldSize   = 0;
	mutable std::mutex m_mutex;
};
