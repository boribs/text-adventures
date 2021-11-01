test:
	@gcc src/parse.c tests/unity/unity.c tests/parse_tests.c -o tests/tests.out
	@./tests/tests.out
