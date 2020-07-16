typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed long s32;
typedef signed long long s64;


#define REG_UNALIGNED ((volatile u64*)0x801)
#define REG_ALIGNED ((volatile u64*)0x800)

#define CON ((volatile u32*)0x4000)

static void __attribute__((noinline)) printout(void) {
	static const char str[] = "Hello, world from MIPS!\n";
	for (unsigned i = 0; i < sizeof(str); i++)
		*CON = str[i];
}

static void __attribute__((noinline)) printhex(u32 x) {
	static const char hexlut[] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};

	*CON = '0';
	*CON = 'x';

	for (unsigned i = 0; i < 8; i++) {
		*CON = hexlut[(x >> 28) & 0xf];
		x <<= 4;
	}

	*CON = '\n';
}

static unsigned printed = 0;

void entry(void)
{
	*REG_ALIGNED = 0x123456789ABCDEF;

	if (!printed) {
		printout();

		u64 data = *REG_UNALIGNED;
		printhex((u32)(data >> 32));
		printhex((u32)data);

		printed = 1;
	}
}
