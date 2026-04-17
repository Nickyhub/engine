#include <stdlib.h>

#include "random.h"

u32 get_random_number() {
	return rand();
}

u32 get_random_number_in_positive_range(unsigned int lowerEnd, unsigned int upperEnd) {
	if (upperEnd - lowerEnd > 0) {
		int a = rand();
		return (a % (upperEnd - lowerEnd) + 1) + lowerEnd;
	}
	return (u32)-1;
}

i32 get_random_number_in_whole_range(i32 lower_end, i32 upper_end) {
	if (upper_end - lower_end > 0) {
		if (lower_end < 0) {
			lower_end *= -1;
			upper_end  += lower_end;
			int a = get_random_number_in_positive_range(lower_end, upper_end);
			return a - lower_end;
		}
		else {
			return get_random_number_in_positive_range(lower_end, upper_end);
		}
	}
	return 0;
}

f32 get_normalized_float() {
	float a = (float)get_random_number_in_positive_range(0, 100000);
	return a / 100000;
}