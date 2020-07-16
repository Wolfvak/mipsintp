#include <cassert>
#include <thread>

#include "MIPS/perf.h"
#include "MIPS/CPU.h"

void runMipsThread(MIPS::CPU *cpu)
{
	while(1) {
		cpu->run();
	}
}

int main(int argc, char const *argv[])
{
	MIPS::CPU mipsCpu;

	assert(argc == 2);

	FILE *fp = fopen(argv[1], "rb");

	unsigned off = 0;
	uint8_t data = 0;
	while(fread(&data, 1, 1, fp) != 0) {
		mipsCpu.storeMemory<uint8_t> (off, &data);
		off++;
	}
	fclose(fp);

	std::thread mipsThread(runMipsThread, &mipsCpu);
	while(1) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << std::endl << mipsCpu << std::endl;
	}

	return 0;
}
