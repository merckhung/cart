CPPLINT	:=	tools/cpplint.py
CPPLINT_FILTER := --filter=-whitespace/line_length,-build/include,-readability/function,-readability/streams,-readability/todo,-runtime/references,-runtime/sizeof,-runtime/threadsafe_fn,-runtime/printf,-readability/casting,-runtime/int
CPPLINT_SRC := $(shell find ./ -name "*.h" -o -name "*.cc" | grep -v tools)

cpplint:
	@$(CPPLINT) $(CPPLINT_FILTER) $(CPPLINT_SRC)

