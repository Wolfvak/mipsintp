static __attribute__((noinline)) unsigned long long fact(unsigned long n)
{
	unsigned long long res = 1;
	for (unsigned i = 1; i <= n; i++)
		res *= i;
	return res;
}

void __attribute__((section(".text.start"))) _start(void)
{
	unsigned i = 0;
	volatile unsigned long long x;
	while(1) {
		x = fact(i++);
	}
}
