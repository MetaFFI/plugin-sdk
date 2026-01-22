#pragma once

#include <string>
#include <vector>
#include <mutex>
#include "python_h_declares.h"

/**
 * Base Entity class (abstract)
 * 
 * All entity types inherit from this. Provides common interface.
 */
class Entity
{
public:
	virtual ~Entity() = default;
	
	/**
	 * Get parameter types
	 */
	virtual const std::vector<PyObject*>& get_params_types() const = 0;
	
	/**
	 * Get return value types
	 */
	virtual const std::vector<PyObject*>& get_retval_types() const = 0;
};

/**
 * Callable Entity base class
 * 
 * Base for Function, Method, Constructor entities
 */
class CallableEntity : public Entity
{
public:
	/**
	 * Call the callable entity with a vector of native PyObject* arguments.
	 * @throws std::exception on failure.
	 */
	PyObject* call(const std::vector<PyObject*>& args);

	/**
	 * Call the callable entity with a native PyObject* tuple.
	 * @throws std::exception on failure.
	 */
	PyObject* call(PyObject* args_tuple);
	
	virtual const std::vector<PyObject*>& get_params_types() const override;
	virtual const std::vector<PyObject*>& get_retval_types() const override;

protected:
	PyObject* m_pyCallable;  // Python callable object
	std::vector<PyObject*> m_paramsTypes;
	std::vector<PyObject*> m_retvalTypes;
	bool m_isVarargs;
	bool m_isNamedArgs;
	mutable std::mutex m_mutex;  // Thread safety
	
	CallableEntity(PyObject* py_callable, 
	               const std::vector<PyObject*>& params_types,
	               const std::vector<PyObject*>& retval_types,
	               bool varargs, bool named_args);
	
	virtual ~CallableEntity();
	
	// RAII: Copy increments refcount, move transfers ownership
	CallableEntity(const CallableEntity& other);
	CallableEntity(CallableEntity&& other) noexcept;
	CallableEntity& operator=(const CallableEntity& other);
	CallableEntity& operator=(CallableEntity&& other) noexcept;
	
	// Helper methods
	std::string check_python_error() const;
};

/**
 * Variable Entity base class
 * 
 * Base for GlobalGetter, GlobalSetter, FieldGetter, FieldSetter entities
 */
class VariableEntity : public Entity
{
public:
	/**
	 * Get variable value (for getters).
	 * Returns a new reference to a PyObject*.
	 * @throws std::exception on failure.
	 */
	virtual PyObject* get() = 0;
	
	/**
	 * Set variable value (for setters).
	 * @throws std::exception on failure.
	 */
	virtual void set(PyObject* value) = 0;
	
	/**
	 * Get variable type (borrowed reference)
	 */
	virtual PyObject* get_type() const = 0;
	
	const std::vector<PyObject*>& get_params_types() const override;
	const std::vector<PyObject*>& get_retval_types() const override;

protected:
	PyObject* m_attributeHolder;  // Object that holds the attribute
	std::string m_attributeName;  // Name of the attribute
	std::vector<PyObject*> m_paramsTypes;
	std::vector<PyObject*> m_retvalTypes;
	mutable std::mutex m_mutex;  // Thread safety
	
	VariableEntity(PyObject* attribute_holder, const std::string& attribute_name,
	               const std::vector<PyObject*>& params_types,
	               const std::vector<PyObject*>& retval_types);
	
	virtual ~VariableEntity();
	
	// RAII: Copy increments refcount, move transfers ownership
	VariableEntity(const VariableEntity& other);
	VariableEntity(VariableEntity&& other) noexcept;
	VariableEntity& operator=(const VariableEntity& other);
	VariableEntity& operator=(VariableEntity&& other) noexcept;
	
	// Helper methods
	std::string check_python_error() const;
};

// Concrete Entity Classes

/**
 * Python Function Entity
 */
class PythonFunction : public CallableEntity
{
public:
	PythonFunction(PyObject* py_function,
	               const std::vector<PyObject*>& params_types,
	               const std::vector<PyObject*>& retval_types,
	               bool varargs, bool named_args);
	
	~PythonFunction() override;
	
};

/**
 * Python Method Entity
 */
class PythonMethod : public CallableEntity
{
public:
	PythonMethod(PyObject* py_method,
	             const std::vector<PyObject*>& params_types,
	             const std::vector<PyObject*>& retval_types,
	             bool varargs, bool named_args, bool instance_required);
	
	~PythonMethod() override;
	
private:
	bool m_instanceRequired;
};

/**
 * Python Constructor Entity
 */
class PythonConstructor : public CallableEntity
{
public:
	PythonConstructor(PyObject* py_constructor,
	                 const std::vector<PyObject*>& params_types,
	                 const std::vector<PyObject*>& retval_types,
	                 bool varargs, bool named_args, bool instance_required);

	~PythonConstructor() override;

private:
	bool m_instanceRequired;
};

/**
 * Python Global Getter Entity
 */
class PythonGlobalGetter : public VariableEntity
{
public:
	PythonGlobalGetter(PyObject* module, const std::string& global_name,
	                  const std::vector<PyObject*>& retval_types);
	
	~PythonGlobalGetter() override;
	
	PyObject* get() override;
	void set(PyObject* value) override;  // Not supported for getter
	
	PyObject* get_type() const override;
};

/**
 * Python Global Setter Entity
 */
class PythonGlobalSetter : public VariableEntity
{
public:
	PythonGlobalSetter(PyObject* module, const std::string& global_name,
	                  const std::vector<PyObject*>& params_types);
	
	~PythonGlobalSetter() override;
	
	PyObject* get() override;  // Not supported for setter
	void set(PyObject* value) override;
	
	PyObject* get_type() const override;
};

/**
 * Python Field Getter Entity
 */
class PythonFieldGetter : public VariableEntity
{
public:
	PythonFieldGetter(PyObject* class_or_instance, const std::string& field_name,
	                 const std::vector<PyObject*>& retval_types);
	
	~PythonFieldGetter() override;
	
	PyObject* get() override;
	void set(PyObject* value) override;  // Not supported for getter
	
	PyObject* get_type() const override;
};

/**
 * Python Field Setter Entity
 */
class PythonFieldSetter : public VariableEntity
{
public:
	PythonFieldSetter(PyObject* class_or_instance, const std::string& field_name,
	                 const std::vector<PyObject*>& params_types);
	
	~PythonFieldSetter() override;
	
	PyObject* get() override;  // Not supported for setter
	void set(PyObject* value) override;
	
	PyObject* get_type() const override;
};
