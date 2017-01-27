/*
 * Copyright (c) 2017 Renesas Electronics Europe Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <limits.h>
#include <unistd.h>

#define NR_CPU_CORES	8

/*
 * Since it reads the idle jiffy counter (where a jiffy=10ms), if we
 * just use the raw value the resolution is rubbish if called too often.
 * So, we add in the decayed previous value at a higher resolution. This
 * allows the function to be called often and get a smooth output for
 * the cpu idle time.
 */
 #define JIFFY_MS	10
static unsigned long long last_idle[NR_CPU_CORES];
static int last_delta[NR_CPU_CORES];
int get_cpu_idle_ms(int core, int decay_n, int decay_d)
{
	char match[] = "cpu0 %llu %llu %llu %llu";
	char line[80];
	FILE *file;
	unsigned long long user, user_low, sys, curr_idle;
	int delta, scaled_delta;

	if (core >= NR_CPU_CORES)
		return -1;
	if (decay_n >= decay_d)
		return -1;

	file = fopen("/proc/stat", "r");
	for (int i = 0; i <= core + 1; i++)
		fgets(line, sizeof line, file);
	fclose(file);

	match[3] += core;
	sscanf(line, match, &user, &user_low, &sys, &curr_idle);

	/* If this is the first call, we will get a massive value for the nr
	 * of idle jiffies, so deal with it */
	if (last_idle[core] == 0) {
		last_idle[core] = curr_idle;
		return 0;
	}

	if (last_idle[core] > curr_idle)
		delta = (int)(ULLONG_MAX - last_idle[core] + curr_idle);
	else
		delta = (int)(curr_idle - last_idle[core]);

	last_idle[core] = curr_idle;

	/* delta in ms */
	delta *= JIFFY_MS;

	/* Measure delta in 1/64ms to give the decayed values some weight */
	delta *= 64;

	/* Decay previous delta */
	last_delta[core] = (last_delta[core] * decay_n) / decay_d;
	last_delta[core] += delta;

	/* Remove the scaling factor that adding the decayed values applies:
	   Factor is (N / (D - N)) + 1 */
	scaled_delta = (last_delta[core] * (decay_d - decay_n)) / decay_d;

	/* Convert time to ms */
	return (scaled_delta / 64);
}
