#include <QtCore/QCoreApplication>
#include <cstdio>
#include <cstdlib>
#include "sealevel.h"

static wrpformat wrp;

seaLevel::seaLevel(int argc, char *argv[])
{
    Open_Files(argc, argv);
    Read_Signature();
    Read_Elevations();
    Read_Textures();
    Write_Elevations();
    Write_Textures();
    Read_Objects();
    Close_Files();
}


void seaLevel::Open_Files(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Not enough parameters!\n\n\tSeaLevel Source.WRP Destination.WRP <SeaLevelAdjust>\n\n");
        exit (1);
    }

    printf ("Source WRP: %s\nDestination WRP: %s\n", argv[1], argv[2]);

    map = fopen (argv[1], "rb");
    if (!map)
    {
        printf ("error in %s\n", argv[1]);
        exit (1);
    }

    printf ("Opened %s\n", argv[1]);

    output = fopen (argv[2], "wb");
    if (!output)
    {
        printf ("error at %s file\n", argv[2]);
        exit (1);
    }

    printf ("Opened %s\n", argv[2]);

    // grab the sea level adjust from cmd line
    SeaLevelAdjust = atoi(argv[3]);
    SeaLevelAdjust = (SeaLevelAdjust * 22);
    printf("SeaLevelAdjust: %i meters\n", (SeaLevelAdjust / 22) );
}


void seaLevel::Read_Signature()
{
    fread (sig,4,1,map);
    sig[4] = 0;
    fwrite (sig, 4, 1, output);

    fread(&MapSize,4,1,map);
    fwrite(&MapSize,4,1,output);

    fread(&MapSize,4,1,map);
    fwrite(&MapSize,4,1,output);

    if (MapSize > 4096)
    {
        printf("MapSize %i is too large! exiting!\n",MapSize);
        exit(1);
    };

    printf("Signature: %s\nMapSize: %i\nReading elevations...",sig,MapSize);
}


void seaLevel::Read_Elevations()
{
    // read elevations
    int x = 0,z = 0;
    for (int zx = 0; zx < MapSize*MapSize; zx++)
    {
        fread(&wrp[x][z].Elevation,sizeof(wrp[x][z].Elevation),1,map);

        x++;
        if (x == MapSize)
        {
            z++; x = 0;
        }
        if (z == MapSize)
        {
            z = 0;
        }
    }

    printf(" Done\nReading Textures...");
}


void seaLevel::Read_Textures()
{
    // read textures IDs
    int x = 0, z = 0;
    TexIndex = 0;

    for (int tx = 0; tx < MapSize*MapSize; tx++)
    {
        wrp[x][z].TexIndex = 0;
        fread(&wrp[x][z].TexIndex,sizeof(wrp[x][z].TexIndex),1,map);

        x++;
        if (x == MapSize)
        {
            z++;
            x = 0;
        }
        if (z == MapSize)
        {
            z = 0;
        }
    }

    printf(" Done\nWriting elevations...");
}


void seaLevel::Write_Elevations()
{
    // write the elevations
    int x = 0, z = 0;
    for (int xx = 0; xx < MapSize*MapSize; xx++)
    {
        // here we check if we are processing sea level data.
        if ( wrp[x][z].Elevation < 0 )
        {
            wrp[x][z].Elevation += SeaLevelAdjust;
            // if sea level suddenly becomes over 0m, truncate it to 0m.
            if ( wrp[x][z].Elevation > 0 )
            {
                wrp[x][z].Elevation = 0;
            }
        }

        fwrite(&wrp[x][z].Elevation,sizeof(wrp[x][z].Elevation),1,output);
        x++;
        if (x == MapSize)
        {
            z++;
            x = 0;
        }
        if (z == MapSize)
        {
            z = 0;
        }
    }

    printf(" Done\nWriting texture index...");
}


void seaLevel::Write_Textures()
{
    // write the texture indexes
    int x = 0, z = 0;
    for (int cr = 0; cr < MapSize*MapSize; cr++)
    {
        fwrite(&wrp[x][z].TexIndex,sizeof(wrp[x][z].TexIndex),1,output);
        x++;
        if (x == MapSize)
        {
            z++;
            x = 0;
        }
        if (z == MapSize)
        {
            z = 0;
        }
    }

    printf(" Done\nReading and writing Texture names...");

    //textures 32 char length and total of 512
    for (int ix = 0; ix < 512; ix++)
    {
        sig[0] = 0;
        fread(sig,32,1,map);
        fwrite(sig,32,1,output);
    }

    printf(" Done\nReading writing 3dObjects...");
}


void seaLevel::Read_Objects()
{
    char dObjName[76],EmptyName[76];
    float dDir,dDirX = 0,dDirZ = 0;
    long dObjIndex = 0;

    // clear the objname variable
    for (int i = 0; i < 76; i++) EmptyName[i] = 0;

    while (!feof(map))
    {
        dDir = 1;
        fread(&dDir,4,1,map);
        fwrite(&dDir,4,1,output);
        dDir = 0;
        fread(&dDir,4,1,map);
        fwrite(&dDir,4,1,output);
        fread(&dDir,4,1,map);
        fwrite(&dDir,4,1,output);
        fread(&dDir,4,1,map);
        fwrite(&dDir,4,1,output);
        dDir = 1;
        fread(&dDir,4,1,map);
        fwrite(&dDir,4,1,output);
        dDir = 0;
        fread(&dDir,4,1,map);
        fwrite(&dDir,4,1,output);
        fread(&dDir,4,1,map);
        fwrite(&dDir,4,1,output);
        fread(&dDir,4,1,map);
        fwrite(&dDir,4,1,output);
        dDir = 1;
        fread(&dDir,4,1,map);
        fwrite(&dDir,4,1,output);

        // X coord
        fread(&dDirX,4,1,map);
        fwrite(&dDirX,4,1,output);

        // here we do our magic for aling to ground
        fread(&dDir,4,1,map);

        // Z coord
        fread(&dDirZ,4,1,map);

        dObjIndex++;
        fread(&dObjIndex,4,1,map);
        strcpy(dObjName, EmptyName);
        fread(dObjName,76,1,map);
        //strlwr(dObjName);

        fwrite(&dDir,4,1,output);
        fwrite(&dDirZ,4,1,output);
        fwrite(&dObjIndex,4,1,output);

        fwrite(dObjName,sizeof(dObjName),1,output);
    };

    printf(" Done\n");
}


void seaLevel::Close_Files()
{
    fclose(map);
    fclose(output);
}
