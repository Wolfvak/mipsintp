static __attribute__((noinline)) unsigned long fact(unsigned int n)
{
	unsigned long res = 1;
	for (unsigned i = 1; i <= n; i++)
		res *= i;
	return res;
}

void __attribute__((section(".text.start"))) _start(void)
{
	volatile unsigned long x;
	for (unsigned i = 0; i < 100000; i++)
		x = fact(i);
	while(1);
}
