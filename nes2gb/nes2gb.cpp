//
// FC-Emulator for GameBoy
//
// m-kami@da2.so-net.ne.jp
// http://www.playoffline.com/
//

#include <stdio.h>

#define	FILENAME	"fc.gb"

void main(int argc, char *argv[])
{
	printf("FC-Emulator for GameBoy\n");

	if (argc != 2)
	{
		printf("usage: nes2gb <nes-file>\n");
		return;
	}

	FILE *fp = fopen(FILENAME, "rb");
	if (fp == NULL)
	{
		printf("error: can't open " FILENAME "\n");
		return;
	}
	unsigned char buf[0x10000];
	fread(buf, 1, sizeof buf, fp);
	fclose(fp);

	fp = fopen(argv[1], "rb");
	if (fp == NULL)
	{
		printf("error: can't open %s\n", argv[1]);
		return;
	}
	unsigned char header[16];
	fread(header, 1, sizeof header, fp);
	if (header[0] != 'N'
	||  header[1] != 'E'
	||  header[2] != 'S'
	||  header[4] != 1
	||  header[5] != 1)
	{
		printf("error: not supported\n");
		fclose(fp);
		return;
	}
	fread(&buf[0x4000], 1, 0x4000, fp);
	fread(&buf[0x8000], 1, 0x2000, fp);
	fclose(fp);

	int crc = 0;
	for (int n = 0; n < 0x10000; n++)
	{
		if (n == 0x14e
		||  n == 0x14f)
		{
			continue;
		}

		crc += buf[n];
	}
	buf[0x14e] = (crc >> 8) & 0xff;
	buf[0x14f] = crc & 0xff;

	fp = fopen(FILENAME, "wb");
	if (fp == NULL)
	{
		printf("error: can't open " FILENAME "\n");
		return;
	}
	fwrite(buf, 1, sizeof buf, fp);
	fclose(fp);

	return;
}
