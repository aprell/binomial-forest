CC := gcc
CFLAGS := -std=c99
CFLAGS += -Wall -Wextra -Wstrict-aliasing=2 -Wno-override-init
CFLAGS += -O2
CFLAGS += -fsanitize=address,undefined

binomial_forest: CPPFLAGS += -DTEST
binomial_forest: binomial_forest.c

binomial_forest_coverage: CPPFLAGS += -DTEST
binomial_forest_coverage: binomial_forest.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -fprofile-arcs -ftest-coverage $< -o $@

test: binomial_forest
	@if ./$<; then \
		printf "\e[1m\e[32mPass\e[0m\n"; \
	else \
		printf "\e[1m\e[31mFail\e[0m\n"; \
	fi

coverage: binomial_forest_coverage
	@./$<
	@gcov binomial_forest.c
	@grep '#####' binomial_forest.c.gcov

clean:
	rm -rf binomial_forest{,_coverage} *.{gcov,gcda,gcno}

.PHONY: test coverage clean
