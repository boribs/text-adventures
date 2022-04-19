test:
	@echo Compiling...
	@gcc src/parse.c tests/unity/unity.c tests/text_adventure_tests.c -o tests/tests.out
	@echo Running...
	@./tests/tests.out

build:
	@gcc adv.c src/parse.c src/adventure.c examples/example.c -o adv
