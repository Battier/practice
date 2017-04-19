#include <stdio.h>
#define NEW_DISK_IDENT 0x00000000
#define IDENT_POSITION 0x1B8
#define IDENT_LENGTH 4 

void readDiskIdentifier(FILE* fp)
{
        fseek(fp, IDENT_LENGTH - 1 + IDENT_POSITION, SEEK_SET);
        for(int i = 0; i < IDENT_LENGTH; i++)
        {
                printf("%0*X", 2, fgetc(fp));
                fseek(fp, -2, SEEK_CUR);
        }
        printf("\n");
}

void writeDiskIdentifier(FILE* fp)
{
        fseek(fp, IDENT_POSITION, SEEK_SET);
        for(int i = 0; i < IDENT_LENGTH; i++)
                fputc(0xFF & (NEW_DISK_IDENT >> i*8), fp);
}

int main(int argc, char **argv)
{
        if (argc > 1)
        {
                printf("%s\n", argv[1]);
                FILE* fp = fopen(argv[1], "r+");
                printf("Current disk identifier: ");
                readDiskIdentifier(fp);
                writeDiskIdentifier(fp);
                printf("New disk identifier: ");
                readDiskIdentifier(fp);
                fclose(fp);
                return 0;
        } else
                printf(" usage: %s file\n Example: %s /dev/sdb\n", argv[0],
                        argv[0]);
                return -2;

}

