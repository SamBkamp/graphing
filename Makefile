
graphics:write.c
	gcc $^ -Wall -Wextra -lpng -lm -o $@

stocks:stocks.c graphics.c
	gcc $^ -Wall -Wextra -lpng -lm -o $@
