test:
	@echo Compiling...
	@gcc src/parse.c tests/unity/unity.c tests/parse_tests.c -o tests/tests.out
	@echo Running...
	@./tests/tests.out
