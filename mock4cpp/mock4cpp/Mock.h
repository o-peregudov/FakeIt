#ifndef Mock_h__
#define Mock_h__

#include <type_traits>
#include <memory>

#include "../mockutils/DynamicProxy.h"
#include "StubbingImpl.h"
#include "mockito_clouses.h"
#include "MockRepository.h"

using namespace mock4cpp;
using namespace mock4cpp::stubbing;
using namespace mock4cpp::verification;

template <typename C>
class Mock : private MockBase
{	
private:
	DynamicProxy<C> instance;

	template <typename R, typename... arglist>
	MethodMock<R, arglist...>& stubMethodIfNotStubbed(DynamicProxy<C> &instance, R(C::*vMethod)(arglist...)){
		if (!instance.isStubbed(vMethod)){
			auto methodMock = new MethodMock<R, arglist...>(*this);
			instance.stubMethod(vMethod, methodMock);
		}
		MethodMock<R, arglist...> * methodMock = instance.getMethodMock<MethodMock<R, arglist...> *>(vMethod);
		return *methodMock;
	}

	template <typename R, typename... arglist>
	class StubbingContextImpl : public StubbingContext<R,arglist...> {
		R(C::*vMethod)(arglist...);
		Mock<C>& mock;
	public:
		StubbingContextImpl(Mock<C>& mock, R(C::*vMethod)(arglist...)) :mock{mock},vMethod{ vMethod }{
		}
		virtual MethodMock<R, arglist...>& getMethodMock() override {
			return mock.stubMethodIfNotStubbed(mock.instance,vMethod);
		};
	};

	template <typename R, typename... arglist, class = typename std::enable_if<!std::is_void<R>::value>::type>
	FunctionStubbingRoot< R, arglist...> StubImpl(R(C::*vMethod)(arglist...)){
		return FunctionStubbingRoot<R, arglist...>(
			std::shared_ptr<StubbingContext <R, arglist...>>(new StubbingContextImpl< R, arglist...>(*this, vMethod)));
	}

	template <typename R, typename... arglist, class = typename std::enable_if<std::is_void<R>::value>::type>
	ProcedureStubbingRoot<R, arglist...> StubImpl(R(C::*vMethod)(arglist...)){
		return ProcedureStubbingRoot<R, arglist...>(
			std::shared_ptr <StubbingContext<R, arglist...>>(new StubbingContextImpl< R, arglist...>(*this, vMethod)));
	}

	void Stub(){}

	template <class MEMBER_TYPE, typename... arglist>
	void stubDataMember(MEMBER_TYPE C::*member, const arglist&... ctorargs)
	{
		instance.stubDataMember(member, ctorargs...);
	}

	template <typename R, typename... arglist>
	void stubMethodInvocation(MethodInvocationMock<R, arglist...> * methodInvocationMock){
		//stubMethodIfNotStubbed()
		//methodInvocationMocks.push_back(methodInvocationMock);
	}

public:
	static_assert(std::is_polymorphic<C>::value, "Can only mock a polymorphic type");

	Mock() : MockBase{}, instance{}{
	}
	
	~Mock(){
	}
	
	C& get()
	{
		return instance.get();
	}

	C& operator()()
	{
		return instance.get();
	}

	template <typename R, typename... arglist, class = typename std::enable_if<!std::is_void<R>::value>::type>
	FunctionStubbingRoot<R, arglist...> When(R(C::*vMethod)(arglist...) const){
		auto methodWithoutConstVolatile = reinterpret_cast<R(C::*)(arglist...)>(vMethod);
		return StubImpl(methodWithoutConstVolatile);
	}

	template < typename R, typename... arglist, class = typename std::enable_if<!std::is_void<R>::value>::type>
	FunctionStubbingRoot<R, arglist...> When(R(C::*vMethod)(arglist...) volatile){
		auto methodWithoutConstVolatile = reinterpret_cast<R(C::*)(arglist...)>(vMethod);
		return StubImpl(methodWithoutConstVolatile);
	}

	template <typename R, typename... arglist, class = typename std::enable_if<!std::is_void<R>::value>::type>
	FunctionStubbingRoot<R, arglist...> When(R(C::*vMethod)(arglist...) const volatile){
		auto methodWithoutConstVolatile = reinterpret_cast<R(C::*)(arglist...)>(vMethod);
		return StubImpl(methodWithoutConstVolatile);
	}

	template <typename R, typename... arglist, class = typename std::enable_if<!std::is_void<R>::value>::type>
	FunctionStubbingRoot<typename R, arglist...> When(R(C::*vMethod)(arglist...)) {
		return StubImpl(vMethod);
	}

	//		

	template <typename R, typename... arglist, class = typename std::enable_if<std::is_void<R>::value>::type>
	ProcedureStubbingRoot<R, arglist...> When(R(C::*vMethod)(arglist...) const){
		auto methodWithoutConstVolatile = reinterpret_cast<R(C::*)(arglist...)>(vMethod);
		return StubImpl(methodWithoutConstVolatile);
	}

	template <typename R, typename... arglist, class = typename std::enable_if<std::is_void<R>::value>::type>
	ProcedureStubbingRoot<R, arglist...> When(R(C::*vMethod)(arglist...) volatile){
		auto methodWithoutConstVolatile = reinterpret_cast<R(C::*)(arglist...)>(vMethod);
		return StubImpl(methodWithoutConstVolatile);
	}

	template <typename R, typename... arglist, class = typename std::enable_if<std::is_void<R>::value>::type>
	ProcedureStubbingRoot<R, arglist...> When(R(C::*vMethod)(arglist...) const volatile){
		auto methodWithoutConstVolatile = reinterpret_cast<R(C::*)(arglist...)>(vMethod);
		return StubImpl(methodWithoutConstVolatile);
	}

	template <typename R, typename... arglist, class = typename std::enable_if<std::is_void<R>::value>::type>
	auto When(R(C::*vMethod)(arglist...)) -> decltype(StubImpl(vMethod)){
		return StubImpl(vMethod);
	}

	template <class DM, typename... arglist
		, class = typename std::enable_if<std::is_member_object_pointer<DM>::value>::type
	>
	void Stub(DM member, const arglist&... ctorargs)
	{
		stubDataMember(member, ctorargs...);
	}

	template <typename H, typename... M
		, class = typename std::enable_if<std::is_member_function_pointer<H>::value>::type
	>
	void Stub(H head, M... tail) 
	{
		When(head).startStubbing();
		Stub(tail...);
	}

	template <
		typename MEMBER
		, class = typename std::enable_if<std::is_member_function_pointer<MEMBER>::value>::type
	>
	auto operator [](MEMBER member) -> decltype(When(member)) {
		return When(member);
	}

};

class VerifyFunctor
{
public:
	VerifyFunctor() {}
	FunctionVerificationProgress& operator() (FunctionVerificationProgress& verificationProgress) {
		verificationProgress.startVerification();
		return  verificationProgress;
	}
} static Verify;

class WhenFunctor
{
public:
	WhenFunctor() {}

	template <typename R, typename... arglist>
	FirstProcedureStubbingProgress<R, arglist...>& operator()(FirstProcedureStubbingProgress<R, arglist...>& stubbingProgress) {
		return  stubbingProgress;
	}

	template <typename R, typename... arglist>
	FirstFunctionStubbingProgress<R, arglist...>& operator()(FirstFunctionStubbingProgress<R, arglist...>& stubbingProgress) {
		return  stubbingProgress;
	}

} static When;

class StubFunctor
{
private:
	void operator() (){}
public:
	StubFunctor() {}

	template <typename H>
	H& operator() (H& head) {
		head.startStubbing();
		return head;
	}

	template <typename H, typename... M>
	void operator() (H& head, M&... tail) {
		head.startStubbing();
		this->operator()(tail...);
	}
} static Stub;



#endif // Mock_h__