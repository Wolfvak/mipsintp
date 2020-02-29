#include "common.h"
#include "mipsint.h"

void m_ctx_init(mips_ctx *ctx)
{
	memset(ctx->r, 0, sizeof(ctx->r));
	memset(ctx->m, 0, sizeof(ctx->m));
	ctx->r[SP] = sizeof(ctx->m);
	ctx->inst_ctr = 0;
}

void m_ctx_dump(const mips_ctx *ctx)
{
	printf("\n");
	for (uint i = 0; i < 32; i++) {
		printf("R%02d: 0x%08X", i, ctx->r[i]);
		if ((i % 4) != 3) printf("   ");
		else printf("\n");
	}

	printf(
		"PC:  0x%08X   HI:  0x%08X   LO:  0x%08X\n\n",
		ctx->r[PC], ctx->r[HI], ctx->r[LO]
	);
	printf("CTR: %llu\n", ctx->inst_ctr);
	fflush(stdout);
}

void m_ctx_load(mips_ctx *ctx, const char *path)
{
	FILE *b = fopen(path, "rb+");
	assert(b != NULL);
	size_t fsz;

	fseek(b, 0L, SEEK_END);
	fsz = ftell(b);
	fseek(b, 0L, SEEK_SET);

	fread(ctx->m, fsz, 1, b);
	fclose(b);
}

void mips_start_exec(mips_ctx *ctx);

void *exec_fn(void *arg) {
	mips_ctx *ctx = arg;
	mips_start_exec(ctx);
	return NULL;
}

#include <pthread.h>
int main(int argc, char *argv[])
{
	pthread_t pth;
	mips_ctx *ctx = malloc(sizeof(mips_ctx));

	m_ctx_init(ctx);

	assert(argc == 2);
	m_ctx_load(ctx, argv[1]);

	pthread_create(&pth, NULL, exec_fn, ctx);

	sleep(1);
	m_ctx_dump(ctx);
	return 0;
}
