#include <stdio.h>
#include "modbuscrc16.h"
#include "x10request.h"

uint8_t X10RequestCheck(X10RequestContext* ctx)
{
	const uint8_t count = X10RequestGetCount(ctx);
	const uint8_t bcount = X10RequestGetBcount(ctx);

	if( X10RequestCalcCrc(ctx) != X10RequestGetCrc(ctx) )
	{
		return 0;
	}
	else if( count != (bcount / 2) )
	{
		return 0;
	}
	else if( count < X10RequestCountMin )
	{
		return 0;
	}
	else if( count > X10RequestCountMax )
	{
		return 0;
	}
	return 1;
}
uint8_t X10RequestGetSlave(X10RequestContext* ctx)
{
	return ctx->data[ X10RequestIndexSlave ];
}
uint8_t X10RequestGetFcode(X10RequestContext* ctx)
{
	return ctx->data[ X10RequestIndexFcode ];
}
uint16_t X10RequestGetOffset(X10RequestContext* ctx)
{
	uint16_t offset = 0;
	offset += (ctx->data[ X10RequestIndexOffset0 ] << 8);
	offset += (ctx->data[ X10RequestIndexOffset1 ] << 0);
	return offset;
}
uint16_t X10RequestGetCount(X10RequestContext* ctx)
{
	uint16_t count = 0;
	count += (ctx->data[ X10RequestIndexCount0 ] << 8);
	count += (ctx->data[ X10RequestIndexCount1 ] << 0);
	return count;
}
uint8_t  X10RequestGetBcount(X10RequestContext* ctx)
{
	return ctx->data[ X10RequestIndexBcount ];
}
uint16_t X10RequestGetData(X10RequestContext* ctx, uint8_t i)
{
	uint16_t data = 0;

	if( i < X10RequestCountMin )
	{
		return data;
	}
	if( i > X10RequestCountMax )
	{
		return data;
	}
	if( --i > X10RequestGetCount(ctx) )
	{
		return data;
	}

	data += ctx->data[ (X10RequestIndexData + (2 * i) + 0) ] << 8;
	data += ctx->data[ (X10RequestIndexData + (2 * i) + 1) ] << 0;

	return data;
}
uint16_t X10RequestCalcCrc(X10RequestContext* ctx)
{
	return CRC16(ctx->data, X10RequestIndexData + X10RequestGetBcount(ctx));
}
uint16_t X10RequestGetCrc(X10RequestContext* ctx)
{
	uint16_t crc = 0;
	uint16_t len = X10RequestGetBcount(ctx);

	crc += (ctx->data[ (X10RequestIndexData + len + 0) ] << 0);
	crc += (ctx->data[ (X10RequestIndexData + len + 1) ] << 8);

	return crc;
}
uint8_t X10RequestGetLength(X10RequestContext* ctx)
{
	return X10RequestIndexData + X10RequestGetBcount(ctx) + 2;
}

void X10RequestSetSlave(X10RequestContext* ctx, uint8_t c)
{
	ctx->data[ X10RequestIndexSlave ] = c;
}
void X10RequestSetFcode(X10RequestContext* ctx, uint8_t c)
{
	ctx->data[ X10RequestIndexFcode ] = c;
}
void X10RequestSetOffset(X10RequestContext* ctx, uint16_t s)
{
	ctx->data[ X10RequestIndexOffset0 ] = (s >> 8);
	ctx->data[ X10RequestIndexOffset1 ] = (s >> 0);
}
void X10RequestSetCount(X10RequestContext* ctx, uint16_t s)
{
	ctx->data[ X10RequestIndexCount0 ] = (s >> 8);
	ctx->data[ X10RequestIndexCount1 ] = (s >> 0);
}
void X10RequestSetBcount(X10RequestContext* ctx, uint8_t c)
{
	ctx->data[ X10RequestIndexBcount ] = c;
}
void X10RequestSetData(X10RequestContext* ctx, uint8_t i, uint16_t s)
{
	uint8_t bcount = (i * 2);

	if( bcount < X10RequestCountMin )
	{
		return;
	}
	else if( bcount > X10RequestCountMax )
	{
		return;
	}
	if( bcount > X10RequestGetBcount(ctx) )
	{
		X10RequestSetCount(ctx, bcount/2);
		X10RequestSetBcount(ctx, bcount);
	}
	ctx->data[ (X10RequestIndexData + (bcount-2)) ] = (s >> 8);
	ctx->data[ (X10RequestIndexData + (bcount-1)) ] = (s >> 0);
}
void X10RequestSetCrc(X10RequestContext* ctx, uint16_t s)
{
	uint8_t len = X10RequestGetBcount(ctx);

	ctx->data[ (X10RequestIndexData + len + 0) ] = (s >> 0);
	ctx->data[ (X10RequestIndexData + len + 1) ] = (s >> 8);
}

void X10RequestShowContext(X10RequestContext* ctx)
{
	uint8_t i = 0;
	uint8_t len = X10RequestGetLength(ctx);

	printf("X10Request:[");
	for(i = 0; i < len; i++)
	{
		printf("%02X ", ctx->data[i]);
	}
	printf("\b].%s\n", X10RequestCheck(ctx) == 1 ? "ok" : "xxxx");
}

#ifdef TEST_X10REQUEST
int main(void)
{
	X10RequestContext ctx;

	X10RequestSetSlave(&ctx, 1);
	X10RequestSetFcode(&ctx, 16);
	X10RequestSetOffset(&ctx, 0);
	X10RequestSetCount(&ctx, 1);
	for(int i = 1; i < 11; i++)
	{
		X10RequestSetData(&ctx, i, i);
	}
	//X10RequestSetData(&ctx, 1, 9);
	X10RequestSetCrc(&ctx, X10RequestCalcCrc(&ctx));

	X10RequestShowContext(&ctx);

	return 0;
}
#endif//TEST_X10REQUEST
